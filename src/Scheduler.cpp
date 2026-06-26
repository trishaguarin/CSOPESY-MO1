// ============================================================================
// Scheduler.cpp — CPU Scheduler Implementation
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Emulating a CPU scheduler"
//   "We need the following: process representation, scheduler
//    representation, and a way to debug/verify the correctness of
//    our chosen scheduling algorithm."
//
// REFERENCE: fcfs-scheduler branch (past activity)
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

    // TODO: Launch core worker threads (one per CPU core)
    //   LESSON REFERENCE: Midterm Review activity
    //     "CPU core is actually a thread worker."
    //   for (int i = 0; i < config.numCpu; ++i)
    //       coreThreads.emplace_back(&Scheduler::coreWorker, this, i);

    // TODO: Launch the scheduler loop thread
    //   schedulerThread = std::thread(&Scheduler::schedulerLoop, this);
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
    //   LESSON REFERENCE: Midterm Review activity (page 68)
    //     Util (%) per core + overall
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
    // TODO: Implement the main scheduling loop
    //
    // MO1 REQUIREMENT: CPU ticks (page 5)
    //   while (running) { cpuTick++; /* schedule */ }
    //
    // LESSON REFERENCE: Midterm Review — FCFS
    //   FCFS is non-preemptive. Processes run to completion once assigned.
    //
    // LESSON REFERENCE: Midterm Review — Round-Robin algorithm
    //   "For each CPU cycle, do the following:
    //    a. Update R if there's any pending process to be scheduled.
    //    b. Select the first process in R to be the candidate.
    //    c. Execute candidate. candidate.C++;
    //    d. If candidate.C == T, then perform #b. Put candidate at end of R.
    //       Otherwise, perform #c."
    //
    // Detailed pseudocode:
    //
    //   while (running) {
    //       cpuTickCounter++;
    //
    //       // ── Batch generation ──
    //       if (batchGenerating && cpuTickCounter % config.batchProcessFreq == 0) {
    //           auto proc = generateProcess();
    //           addProcess(proc);
    //       }
    //
    //       // ── Handle sleeping processes ──
    //       // For each process in WAITING state, call tickSleep()
    //       // If it wakes up (WAITING → READY), re-add to readyQueue
    //
    //       // ── Assign ready processes to idle cores ──
    //       // (same logic for both FCFS and RR — the difference is in
    //       //  how coreWorker handles quantum preemption)
    //       // while (!readyQueue.empty() && hasIdleCore()) {
    //       //     auto proc = readyQueue.front();
    //       //     readyQueue.pop();
    //       //     int core = getIdleCore();
    //       //     coreStatus[core] = true;
    //       //     coreProcess[core] = proc;
    //       //     proc->setState(Process::RUNNING);
    //       //     proc->setAssignedCore(core);
    //       //     coreCV.notify_all();
    //       // }
    //
    //       // ── RR preemption check ──
    //       // if (config.schedulerAlgo == "rr") {
    //       //     for each busy core:
    //       //         if coreTicksUsed[core] >= config.quantumCycles {
    //       //             preempt: set process state to READY
    //       //             re-add to readyQueue
    //       //             coreStatus[core] = false
    //       //             coreTicksUsed[core] = 0
    //       //         }
    //       // }
    //
    //       // Small sleep to prevent busy-spinning the host CPU
    //       // std::this_thread::sleep_for(std::chrono::microseconds(100));
    //   }
}

// ── Core Worker ──────────────────────────────────────────────────────────────

void Scheduler::coreWorker(int coreId)
{
    // TODO: Implement core worker thread
    //
    // REFERENCE: fcfs-scheduler branch coreWorker()
    //   Old version: wait for process assignment → execute prints → signal idle
    //
    // LESSON REFERENCE: Midterm Review — "CPU core is actually a thread worker"
    //
    // New version pseudocode:
    //
    //   while (running) {
    //       // Wait for a process to be assigned to this core
    //       {
    //           std::unique_lock<std::mutex> lock(coreMutex);
    //           coreCV.wait(lock, [&] {
    //               return !running || (coreStatus[coreId] && coreProcess.count(coreId));
    //           });
    //           if (!running) break;
    //           proc = coreProcess[coreId];
    //       }
    //
    //       // Execute commands using the ICommand pattern
    //       while (proc && !proc->isFinished()) {
    //           // Handle delays-per-exec (busy waiting)
    //           //   MO1 REQUIREMENT: "The delay is a 'busy-waiting' scheme
    //           //   wherein the process remains in the CPU."
    //           for (uint32_t d = 0; d < config.delaysPerExec; d++) {
    //               // busy wait — process occupies CPU but does nothing
    //           }
    //
    //           // Execute one command via ICommand interface
    //           proc->executeCurrentCommand(coreId);
    //           proc->moveToNextLine();
    //
    //           // Check if process went to WAITING (SLEEP command)
    //           if (proc->getState() == Process::WAITING) {
    //               break; // process gives up CPU
    //           }
    //
    //           // For RR: check if quantum exhausted
    //           // if (config.schedulerAlgo == "rr") {
    //           //     coreTicksUsed[coreId]++;
    //           //     if (coreTicksUsed[coreId] >= config.quantumCycles)
    //           //         break; // preempted
    //           // }
    //       }
    //
    //       // Core is free — signal scheduler
    //       {
    //           std::lock_guard<std::mutex> lock(coreMutex);
    //           coreStatus[coreId] = false;
    //           coreProcess.erase(coreId);
    //       }
    //       schedulerCV.notify_all();
    //   }
}
