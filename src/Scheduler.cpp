// ============================================================================
// Scheduler.cpp — CPU Scheduler Implementation
// ============================================================================
// MO1 REQUIREMENT: Real-time CPU scheduler with FCFS and RR support
//
// REFERENCE: FCFS-scheduler/Scheduler.h
//   The old implementation had:
//   - schedulerWorker(): wait for idle core, pop from queue, assign
//   - coreWorker(): execute prints until finished, signal idle
//   That logic is a good starting point for FCFS mode.
//   For RR, add quantum-based preemption.
//   Replace sleep_for with CPU tick counting.
// ============================================================================

#include "Scheduler.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <thread>
#include <mutex>

Scheduler::Scheduler(const SystemConfig& config)
    : config(config)
{
    coreStatus.assign(config.numCpu, false); // all cores idle
}

Scheduler::~Scheduler()
{
    stop();
}

// ── Lifecycle ────────────────────────────────────────────────────────────────

void Scheduler::start()
{
    running = true;
    schedulerThread = std::thread(&Scheduler::schedulerLoop, this);
    for (int i = 0; i < config.numCpu; ++i)
        coreThreads.emplace_back(&Scheduler::coreWorker, this, i);
}

void Scheduler::stop()
{
    running = false;
    batchGenerating = false;
    schedulerCV.notify_all();
    coreCV.notify_all();

    if (schedulerThread.joinable())
        schedulerThread.join();

    for (auto& t : coreThreads)
        if (t.joinable())
            t.join();
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
    // TODO: Notify scheduler loop when a new process is added.
}

// ── Batch Generation ─────────────────────────────────────────────────────────

void Scheduler::startBatchGeneration()
{
    // TODO: Set batchGenerating = true
    //   The schedulerLoop should check this flag and generate a new process
    //   every config.batchProcessFreq CPU ticks.
    // DONE: Batch generation enabled
    batchGenerating = true;
}

void Scheduler::stopBatchGeneration()
{
    // DONE: Batch generation disabled
    batchGenerating = false;
}

// ── Query Methods ────────────────────────────────────────────────────────────

std::vector<std::shared_ptr<Process>> Scheduler::getRunningProcesses() const
{
    std::lock_guard<std::mutex> lock(listMutex);
    std::vector<std::shared_ptr<Process>> result;
    for (auto& p : allProcesses)
        if (p->getState() == Process::RUNNING)
            result.push_back(p);
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

int Scheduler::getNumCores() const { return config.numCpu; }

int Scheduler::getCoresUsed() const
{
    return static_cast<int>(std::count(coreStatus.begin(), coreStatus.end(), true));
}

int Scheduler::getCoresAvailable() const
{
    return getNumCores() - getCoresUsed();
}

float Scheduler::getCpuUtilization() const
{
    // TODO: Calculate CPU utilization
    //   Simple approach: (cores used / total cores) * 100
    //   Better approach: track cumulative busy ticks / total ticks per core
    // DONE: Simple utilization calculation implemented
    int used = getCoresUsed();
    if (config.numCpu == 0) return 0.0f;
    return (static_cast<float>(used) / config.numCpu) * 100.0f;
}

uint64_t Scheduler::getCpuTicks() const
{
    return cpuTickCounter.load();
}

std::shared_ptr<Process> Scheduler::generateProcess()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> instructionCount(config.minIns, config.maxIns);
    std::uniform_int_distribution<int> opcode(0, 2);
    std::uniform_int_distribution<int> sleepValue(1, 3);

    ++processCounter;
    std::ostringstream name;
    name << "p" << std::setw(2) << std::setfill('0') << processCounter;
    std::vector<Instruction> instructions;
    int count = instructionCount(rng);
    for (int i = 0; i < count; ++i)
    {
        switch (opcode(rng))
        {
            case 0:
                instructions.push_back(Instruction::makePrint("Hello world from " + name.str() + "!"));
                break;
            case 1:
                instructions.push_back(Instruction::makeSleep(static_cast<uint32_t>(sleepValue(rng))));
                break;
            default:
                instructions.push_back(Instruction::makeAdd("x", "1", "2"));
                break;
        }
    }

    return std::make_shared<Process>(processCounter, name.str(), instructions);
}

bool Scheduler::hasIdleCore() const
{
    for (bool busy : coreStatus)
    {
        if (!busy)
            return true;
    }
    return false;
}

int Scheduler::getIdleCore() const
{
    for (int i = 0; i < static_cast<int>(coreStatus.size()); ++i)
    {
        if (!coreStatus[i])
            return i;
    }
    return -1;
}

// ── Scheduler Loop ───────────────────────────────────────────────────────────

void Scheduler::schedulerLoop()
{
    while (running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        uint64_t currentTick = cpuTickCounter.fetch_add(1) + 1;

        if (batchGenerating && config.batchProcessFreq > 0 &&
            (currentTick % config.batchProcessFreq) == 0)
        {
            auto proc = generateProcess();
            addProcess(proc);
        }

        {
            std::lock_guard<std::mutex> lock(listMutex);
            for (auto& proc : allProcesses)
            {
                if (proc->getState() == Process::WAITING)
                {
                    proc->tickSleep();
                }
            }
        }

        if (config.schedulerAlgo == "fcfs")
        {
            while (true)
            {
                std::shared_ptr<Process> proc;
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    if (readyQueue.empty())
                        break;
                    proc = readyQueue.front();
                    readyQueue.pop();
                }

                int core = -1;
                {
                    std::lock_guard<std::mutex> lock(coreMutex);
                    if (!hasIdleCore())
                    {
                        std::lock_guard<std::mutex> queueLock(queueMutex);
                        readyQueue.push(proc);
                        break;
                    }
                    core = getIdleCore();
                    coreStatus[core] = true;
                    coreProcess[core] = proc;
                }

                proc->setState(Process::RUNNING);
                proc->setAssignedCore(core);
                coreCV.notify_all();
            }
        }
        else if (config.schedulerAlgo == "rr")
        {
            // TODO: Implement RR preemption using quantum-cycles.
            //       This should requeue processes once their time slice expires.
        }
    }
}

// ── Core Worker ──────────────────────────────────────────────────────────────

void Scheduler::coreWorker(int coreId)
{
    while (running)
    {
        std::shared_ptr<Process> proc;
        {
            std::unique_lock<std::mutex> lock(coreMutex);
            coreCV.wait(lock, [&] {
                return !running || (coreStatus[coreId] && coreProcess.count(coreId));
            });

            if (!running)
                break;

            auto it = coreProcess.find(coreId);
            if (it != coreProcess.end())
                proc = it->second;
        }

        if (!proc)
            continue;

        while (running && proc && !proc->isFinished())
        {
            bool executed = proc->executeNextInstruction(coreId);
            if (!executed)
                break;

            if (proc->getState() == Process::WAITING)
                break;

            if (config.schedulerAlgo == "rr")
            {
                // TODO: Track per-core quantum usage and preempt when exhausted.
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        {
            std::lock_guard<std::mutex> lock(coreMutex);
            coreStatus[coreId] = false;
            coreProcess.erase(coreId);
        }
        schedulerCV.notify_all();
    }
}
