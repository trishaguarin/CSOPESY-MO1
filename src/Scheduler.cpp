// ============================================================================
// Scheduler.cpp — CPU Scheduler Implementation
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Emulating a CPU scheduler"
//   "We need the following: process representation, scheduler
//    representation, and a way to debug/verify the correctness of
//    our chosen scheduling algorithm."
//
// REFERENCE: fcfs-scheduler branch (past activity)
//   The old implementation had schedulerWorker + coreWorker pattern.
//   That logic is extended here with: RR preemption, CPU tick model,
//   batch generation, delays-per-exec, and config-driven parameters.
// ============================================================================

#include "Scheduler.h"
#include "PrintCommand.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>

Scheduler::Scheduler(const SystemConfig& config)
    : config(config)
{
    coreStatus.assign(config.numCpu, false);
}

Scheduler::~Scheduler()
{
    stop();
}

// ── Lifecycle ────────────────────────────────────────────────────────────────

void Scheduler::start()
{
    if (running.load()) return;
    running = true;

    // Launch core worker threads
    for (int i = 0; i < config.numCpu; ++i)
    {
        coreTicksUsed[i] = 0;
        coreThreads.emplace_back(&Scheduler::coreWorker, this, i);
    }

    // Launch scheduler loop thread
    schedulerThread = std::thread(&Scheduler::schedulerLoop, this);
}

void Scheduler::stop()
{
    running = false;
    batchGenerating = false;
    schedulerCV.notify_all();
    coreCV.notify_all();

    if (schedulerThread.joinable()) schedulerThread.join();
    for (auto& t : coreThreads)
        if (t.joinable()) t.join();
    coreThreads.clear();
}

// ── Process Management ───────────────────────────────────────────────────────

void Scheduler::addProcess(std::shared_ptr<Process> proc)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(proc);
    }
    {
        std::lock_guard<std::mutex> lock(listMutex);
        allProcesses.push_back(proc);
    }
    schedulerCV.notify_all();
}

// ── Batch Generation ─────────────────────────────────────────────────────────

void Scheduler::startBatchGeneration()
{
    batchGenerating = true;
}

void Scheduler::stopBatchGeneration()
{
    batchGenerating = false;
}

// ── Query Methods ────────────────────────────────────────────────────────────

std::vector<std::shared_ptr<Process>> Scheduler::getRunningProcesses() const
{
    std::lock_guard<std::mutex> lock(listMutex);
    std::vector<std::shared_ptr<Process>> result;
    for (auto& p : allProcesses)
    {
        auto s = p->getState();
        if (s == Process::RUNNING || s == Process::WAITING)
            result.push_back(p);
    }
    return result;
}

std::vector<std::shared_ptr<Process>> Scheduler::getFinishedProcesses() const
{
    std::lock_guard<std::mutex> lock(listMutex);
    std::vector<std::shared_ptr<Process>> result;
    for (auto& p : allProcesses)
        if (p->getState() == Process::FINISHED)
            result.push_back(p);
    return result;
}

std::vector<std::shared_ptr<Process>> Scheduler::getAllProcesses() const
{
    std::lock_guard<std::mutex> lock(listMutex);
    return allProcesses;
}

std::shared_ptr<Process> Scheduler::findProcess(const std::string& name) const
{
    std::lock_guard<std::mutex> lock(listMutex);
    for (auto& p : allProcesses)
        if (p->getName() == name)
            return p;
    return nullptr;
}

int Scheduler::getNumCores() const { return config.numCpu; }

int Scheduler::getCoresUsed() const
{
    std::lock_guard<std::mutex> lock(coreMutex);
    return static_cast<int>(std::count(coreStatus.begin(), coreStatus.end(), true));
}

int Scheduler::getCoresAvailable() const
{
    return getNumCores() - getCoresUsed();
}

float Scheduler::getCpuUtilization() const
{
    uint64_t busy = totalBusyTicks.load();
    uint64_t idle = totalIdleTicks.load();
    uint64_t total = busy + idle;
    if (total == 0) return 0.0f;
    return (static_cast<float>(busy) / static_cast<float>(total)) * 100.0f;
}

uint64_t Scheduler::getCpuTicks() const
{
    return cpuTickCounter.load();
}

// ── Scheduler Loop ───────────────────────────────────────────────────────────

void Scheduler::schedulerLoop()
{
    while (running.load())
    {
        cpuTickCounter++;

        // ── Batch generation ──
        if (batchGenerating.load())
        {
            if (cpuTickCounter.load() % config.batchProcessFreq == 0)
            {
                auto proc = generateProcess();
                // Add to ready queue but not to allProcesses again (generateProcess already does addProcess-like work)
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    readyQueue.push(proc);
                }
                {
                    std::lock_guard<std::mutex> lock(listMutex);
                    allProcesses.push_back(proc);
                }
            }
        }

        // ── Handle sleeping processes ──
        {
            std::lock_guard<std::mutex> lock(listMutex);
            for (auto& p : allProcesses)
            {
                if (p->getState() == Process::WAITING)
                {
                    p->tickSleep();
                    // If it woke up, re-add to ready queue
                    if (p->getState() == Process::READY)
                    {
                        std::lock_guard<std::mutex> qlock(queueMutex);
                        readyQueue.push(p);
                    }
                }
            }
        }

        // ── Assign ready processes to idle cores ──
        {
            std::lock_guard<std::mutex> lock(coreMutex);
            for (int i = 0; i < config.numCpu; ++i)
            {
                if (!coreStatus[i])
                {
                    std::lock_guard<std::mutex> qlock(queueMutex);
                    if (readyQueue.empty()) break;

                    auto proc = readyQueue.front();
                    readyQueue.pop();

                    proc->setState(Process::RUNNING);
                    proc->setAssignedCore(i);
                    coreStatus[i] = true;
                    coreProcess[i] = proc;
                    coreTicksUsed[i] = 0;
                }
            }
        }
        coreCV.notify_all();

        // ── Track utilization ──
        {
            std::lock_guard<std::mutex> lock(coreMutex);
            for (int i = 0; i < config.numCpu; ++i)
            {
                if (coreStatus[i])
                    totalBusyTicks++;
                else
                    totalIdleTicks++;
            }
        }

        // Small sleep to control tick rate and prevent host CPU hogging
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// ── Core Worker ──────────────────────────────────────────────────────────────

void Scheduler::coreWorker(int coreId)
{
    while (running.load())
    {
        std::shared_ptr<Process> proc;

        // Wait for process assignment
        {
            std::unique_lock<std::mutex> lock(coreMutex);
            coreCV.wait(lock, [&] {
                return !running.load() ||
                       (coreStatus[coreId] && coreProcess.count(coreId));
            });
            if (!running.load()) break;
            proc = coreProcess[coreId];
        }

        if (!proc) continue;

        // Execute commands
        uint32_t ticksUsed = 0;
        while (proc && !proc->isFinished() && running.load())
        {
            // Busy-wait delay (delays-per-exec)
            // Process stays on CPU but does no work
            bool preempted = false;
            for (uint32_t d = 0; d < config.delaysPerExec; ++d)
            {
                ticksUsed++;
                // RR: check quantum during delay too
                if (config.schedulerAlgo == "rr" && ticksUsed >= config.quantumCycles)
                {
                    preempted = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if (preempted)
                break;

            // Execute one command
            proc->executeCurrentCommand(coreId);
            ticksUsed++;

            // Check if process went to WAITING (SLEEP command)
            // FIX: state is updated to WAITING inside executeCurrentCommand/setSleepTicks.
            // Only clear assignedCore AFTER state is confirmed changed to avoid Core:-1 race.
            if (proc->getState() == Process::WAITING)
            {
                // State is already WAITING — safe to clear core now
                proc->setAssignedCore(-1);
                break;
            }

            proc->moveToNextLine();

            // Check if finished after moveToNextLine
            // state is now FINISHED — safe to clear core
            if (proc->isFinished())
            {
                proc->setAssignedCore(-1);
                break;
            }

            // RR: check quantum after instruction execution
            if (config.schedulerAlgo == "rr" && ticksUsed >= config.quantumCycles)
            {
                break; // preempt — do NOT clear core here; cleanup block handles it
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Core is now free — cleanup
        // FIX: Re-queue the process BEFORE marking the core as free.
        // This closes the window where coreStatus[i]==false but the process
        // isn't in the ready queue yet, which caused "Cores available" to
        // flicker incorrectly during screen -ls.
        {
            // RR preemption: if process not finished and not sleeping, re-queue first
            if (proc && !proc->isFinished() && proc->getState() != Process::WAITING)
            {
                proc->setState(Process::READY);
                proc->setAssignedCore(-1);
                {
                    std::lock_guard<std::mutex> qlock(queueMutex);
                    readyQueue.push(proc); // ← re-queue BEFORE freeing core
                }
            }

            // Now mark core as free (process is already re-queued or terminal)
            std::lock_guard<std::mutex> lock(coreMutex);
            coreStatus[coreId] = false;
            coreProcess.erase(coreId);
            coreTicksUsed[coreId] = 0;
        }
        schedulerCV.notify_all();
    }
}

// ── Process Generator ────────────────────────────────────────────────────────

std::shared_ptr<Process> Scheduler::generateProcess()
{
    processCounter++;

    // Name format: p01, p02, ..., p10, p100, etc.
    std::ostringstream nameStream;
    nameStream << "p" << std::setw(2) << std::setfill('0') << processCounter;
    std::string name = nameStream.str();

    // Random instruction count between min-ins and max-ins
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(config.minIns, config.maxIns);
    uint32_t numInstructions = dist(rng);

    auto proc = std::make_shared<Process>(processCounter, name);

    // Fill with PrintCommand (default: "Hello world from <name>!")
    for (uint32_t i = 0; i < numInstructions; ++i)
    {
        proc->addCommand(std::make_shared<PrintCommand>());
    }

    return proc;
}
