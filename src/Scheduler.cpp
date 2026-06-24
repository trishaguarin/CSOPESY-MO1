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
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>

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
    // TODO: Set batchGenerating = true
    //   The schedulerLoop should check this flag and generate a new process
    //   every config.batchProcessFreq CPU ticks.
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
    std::lock_guard<std::mutex> lock(coreMutex);
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
    int used = getCoresUsed();
    if (config.numCpu == 0) return 0.0f;
    return (static_cast<float>(used) / config.numCpu) * 100.0f;
}

uint64_t Scheduler::getCpuTicks() const
{
    return cpuTickCounter.load();
}

// ── Scheduler Loop ───────────────────────────────────────────────────────────

void Scheduler::schedulerLoop()
{
    // TODO: Implement the main scheduling loop
    //
    // REFERENCE pseudocode from MO1SPECS (page 5):
    //   while (running) {
    //       cpuTick++;
    //       // ... schedule ...
    //   }
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
    //       // ── FCFS: Assign ready processes to idle cores ──
    //       if (config.schedulerAlgo == "fcfs") {
    //           while (!readyQueue.empty() && hasIdleCore()) {
    //               auto proc = readyQueue.front();
    //               readyQueue.pop();
    //               int core = getIdleCore();
    //               coreStatus[core] = true;
    //               coreProcess[core] = proc;
    //               proc->setState(Process::RUNNING);
    //               proc->setAssignedCore(core);
    //               coreCV.notify_all();
    //           }
    //       }
    //
    //       // ── RR: Same as FCFS but also check quantum preemption ──
    //       if (config.schedulerAlgo == "rr") {
    //           // For each busy core:
    //           //   if coreTicksUsed[core] >= config.quantumCycles {
    //           //       preempt: move process back to readyQueue
    //           //       coreStatus[core] = false;
    //           //       coreTicksUsed[core] = 0;
    //           //   }
    //           // Then assign from readyQueue same as FCFS
    //       }
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
    // REFERENCE: FCFS-scheduler/Scheduler.h coreWorker()
    //   Old version:
    //     wait for process assignment → execute prints → signal idle
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
    //       // Execute instructions
    //       while (proc && !proc->isFinished()) {
    //           // Handle delays-per-exec (busy waiting)
    //           // The process stays in the CPU but doesn't execute
    //           // for config.delaysPerExec ticks
    //           for (uint32_t d = 0; d < config.delaysPerExec; d++) {
    //               // busy wait — process occupies CPU but does nothing
    //               // cpuTickCounter is incremented by schedulerLoop
    //           }
    //
    //           // Execute one instruction
    //           bool executed = proc->executeNextInstruction(coreId);
    //
    //           if (proc->getState() == Process::WAITING) {
    //               // SLEEP instruction — process gives up CPU
    //               break;
    //           }
    //
    //           // For RR: check if quantum exhausted
    //           // if (config.schedulerAlgo == "rr") {
    //           //     coreTicksUsed[coreId]++;
    //           //     if (coreTicksUsed[coreId] >= config.quantumCycles)
    //           //         break; // preempted by scheduler
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
