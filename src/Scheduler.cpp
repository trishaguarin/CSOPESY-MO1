#include "Scheduler.h"
#include "PrintCommand.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>

#include "DeclareCommand.h"
#include "AddCommand.h"
#include "SleepCommand.h"

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
    // Set lastBatchTime so the first process is generated immediately
    lastBatchTime = std::chrono::steady_clock::now() -
        std::chrono::seconds(config.batchProcessFreq);
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
    std::lock_guard<std::mutex> lock(windowMutex);
    if (utilizationWindow.empty()) return 0.0f;

    double sum = 0.0;
    for (float v : utilizationWindow)
        sum += v;
    return static_cast<float>(sum / utilizationWindow.size());
}

uint64_t Scheduler::getCpuTicks() const
{
    return cpuTickCounter.load();
}

// ── Scheduler Loop ───────────────────────────────────────────────────────────

void Scheduler::schedulerLoop()
{
    lastBatchTime = std::chrono::steady_clock::now();

    while (running.load())
    {
        cpuTickCounter++;

        // ── Batch generation (time-based) ──
        if (batchGenerating.load())
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - lastBatchTime).count();
            if (elapsed >= static_cast<long long>(config.batchProcessFreq))
            {
                lastBatchTime = now;
                auto proc = generateProcess();
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

        // ── Assign ready processes and Track utilization ──
        {
            std::lock_guard<std::mutex> lock(coreMutex);
            
            // 1. Assign ready processes to idle cores
            for (int i = 0; i < config.numCpu; ++i)
            {
                if (!coreStatus[i])
                {
                    std::lock_guard<std::mutex> qlock(queueMutex);
                    if (readyQueue.empty()) break;

                    auto proc = readyQueue.front();
                    readyQueue.pop();

                    proc->setAssignedCore(i);          // 1. record which core
                    coreStatus[i] = true;              // 2. mark core busy
                    coreProcess[i] = proc;             // 3. register mapping
                    coreTicksUsed[i] = 0;
                    proc->setState(Process::RUNNING);  // 4. NOW make it visible as RUNNING
                }
            }
            
            // 2. Track utilization (sliding window) while we still hold the lock
            int busyCores = 0;
            for (int i = 0; i < config.numCpu; ++i)
            {
                if (coreStatus[i]) busyCores++;
            }
            float utilPct = (config.numCpu > 0)
                ? (static_cast<float>(busyCores) / config.numCpu) * 100.0f
                : 0.0f;

            std::lock_guard<std::mutex> wlock(windowMutex);
            utilizationWindow.push_back(utilPct);
            if ((int)utilizationWindow.size() > UTIL_WINDOW_SIZE)
                utilizationWindow.pop_front();
        }
        coreCV.notify_all();

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


            if (proc->getState() == Process::WAITING)
            {
                // State is already WAITING — safe to clear core now
                proc->setAssignedCore(-1);
                break;
            }

            proc->moveToNextLine();

            // Check if finished after moveToNextLine
            if (proc->isFinished())
            {
                proc->setAssignedCore(-1);
                break;
            }

            // RR: check quantum after instruction execution
            if (config.schedulerAlgo == "rr" && ticksUsed >= config.quantumCycles)
            {
                break; 
            }
        }

        // Sleep once per quantum to simulate CPU time and keep core visibly busy
        // (Avoids per-instruction sleep which suffers from Windows ~15ms granularity)
        if (config.delaysPerExec == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(config.quantumCycles));
        }

    
        {
            
            if (proc && !proc->isFinished() && proc->getState() != Process::WAITING)
            {
                proc->setState(Process::READY);
                proc->setAssignedCore(-1);
                {
                    std::lock_guard<std::mutex> qlock(queueMutex);
                    readyQueue.push(proc); // ← re-queue BEFORE freeing core
                }
            }

            
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
    nameStream << "process" << std::setw(2) << std::setfill('0') << processCounter;
    std::string name = nameStream.str();

    // Random instruction count between min-ins and max-ins
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(config.minIns, config.maxIns);
    uint32_t numInstructions = dist(rng);

    auto proc = std::make_shared<Process>(processCounter, name);

    std::uniform_int_distribution<int> typeDist(0, 3);
    std::uniform_int_distribution<int> valDist(0, 65535);
    std::uniform_int_distribution<int> sleepDist(1, 255);
    std::uniform_int_distribution<int> repeatDist(2, 5);

    for (uint32_t i = 0; i < numInstructions; ++i)
    {
        int type = typeDist(rng);
        switch (type)
        {
            case 0:
                proc->addCommand(std::make_shared<PrintCommand>());
                break;
            case 1:
                proc->addCommand(std::make_shared<DeclareCommand>("x", valDist(rng)));
                break;
            case 2:
                proc->addCommand(std::make_shared<AddCommand>("x", "x", std::to_string(valDist(rng))));
                break;
            case 3:
                //proc->addCommand(std::make_shared<SleepCommand>(sleepDist(rng)));
                proc->addCommand(std::make_shared<DeclareCommand>("x", valDist(rng)));
                break;
        }
    }

    return proc;
}
