// ============================================================================
// Scheduler.h — CPU Scheduler (FCFS & Round-Robin)
// ============================================================================
// MO1 REQUIREMENT: The scheduler (page 4-5)
//   "Your CPU scheduler is real-time and will continuously schedule processes
//    as long as your console is alive."
//   "The scheduler algorithm will be set through the 'initialize' command
//    and through the config.txt file."
//
// MO1 REQUIREMENT: CPU ticks (page 5)
//   "Assume that the CPU tick is an integer counter that tallies the number
//    of frame passes."
//   Pseudocode from spec:
//     while (running) { cpuTick++; /* schedule */ }
//
// MO1 REQUIREMENT: scheduler-start / scheduler-stop (page 4)
//   "scheduler-start: Every X CPU ticks, a new process is generated and
//    put into the ready queue. X = batch-process-freq from config.txt."
//   "scheduler-stop: Stops generating dummy processes."
//
// MO1 REQUIREMENT: FCFS scheduling
//   First-Come-First-Serve: processes run to completion on assigned core.
//
// MO1 REQUIREMENT: Round-Robin scheduling
//   "quantum-cycles: The time slice given for each processor if a
//    round-robin scheduler is used."
//
// MO1 REQUIREMENT: delays-per-exec (page 5)
//   "Delay before executing the next instruction in CPU cycles.
//    The delay is a 'busy-waiting' scheme wherein the process remains
//    in the CPU."
//
// REFERENCE: FCFS-scheduler/Scheduler.h
//   Reusable: worker thread model, ready queue, core assignment logic
//   Must add:  RR preemption, CPU tick model, batch generation,
//              delays-per-exec, config-driven parameters
// ============================================================================
#pragma once

#include "Process.h"
#include "ConfigParser.h"

#include <queue>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>

class Scheduler
{
public:
    explicit Scheduler(const SystemConfig& config);
    ~Scheduler();

    // ── Lifecycle ────────────────────────────────────────────────────────
    void start();  // launch scheduler thread + core worker threads
    void stop();   // signal all threads to stop and join

    // ── Process Management ───────────────────────────────────────────────
    void addProcess(std::shared_ptr<Process> proc);

    // ── Batch Generation ─────────────────────────────────────────────────
    // MO1 REQUIREMENT: scheduler-start / scheduler-stop
    void startBatchGeneration();
    void stopBatchGeneration();

    // ── Query ────────────────────────────────────────────────────────────
    // Used by screen -ls and report-util
    std::vector<std::shared_ptr<Process>> getRunningProcesses()  const;
    std::vector<std::shared_ptr<Process>> getFinishedProcesses() const;
    std::vector<std::shared_ptr<Process>> getAllProcesses()      const;

    int  getNumCores()      const;
    int  getCoresUsed()     const;
    int  getCoresAvailable() const;

    // TODO: CPU utilization calculation
    //   CPU utilization % = (cores used / total cores) * 100
    //   Or more accurately: track ticks where cores were busy vs idle
    float getCpuUtilization() const;

    uint64_t getCpuTicks() const;

private:
    // ── Scheduler Thread ─────────────────────────────────────────────────
    // TODO: Implement the scheduler loop
    //   The main scheduler thread that:
    //   1. Increments CPU tick counter each iteration
    //   2. Checks for idle cores
    //   3. Assigns ready processes to idle cores
    //   4. For RR: checks if running process has exceeded quantum
    //   5. For batch generation: creates new process every batch-process-freq ticks
    void schedulerLoop();

    // ── Core Worker Threads ──────────────────────────────────────────────
    // TODO: Each core worker:
    //   1. Waits for a process assignment
    //   2. Executes instructions one at a time
    //   3. Between instructions, busy-waits for delays-per-exec ticks
    //   4. For RR: yields after quantum-cycles ticks
    //   5. When process finishes or is preempted, signals scheduler
    void coreWorker(int coreId);

    // ── Batch Process Generator ──────────────────────────────────────────
    // TODO: Generate a new process with:
    //   - Human-readable name (p01, p02, ..., p1240)
    //   - Random instruction count between min-ins and max-ins
    //   - Randomized instruction types
    // std::shared_ptr<Process> generateProcess();

    // ── Configuration ────────────────────────────────────────────────────
    SystemConfig config;

    // ── State ────────────────────────────────────────────────────────────
    std::atomic<bool>     running{false};
    std::atomic<bool>     batchGenerating{false};
    std::atomic<uint64_t> cpuTickCounter{0};

    // ── Process Queues ───────────────────────────────────────────────────
    std::queue<std::shared_ptr<Process>>   readyQueue;
    std::vector<std::shared_ptr<Process>>  allProcesses;

    // ── Threading ────────────────────────────────────────────────────────
    std::thread              schedulerThread;
    std::vector<std::thread> coreThreads;

    // ── Per-Core State ───────────────────────────────────────────────────
    std::vector<bool>                              coreStatus;   // true = busy
    std::map<int, std::shared_ptr<Process>>        coreProcess;  // coreId → process

    // TODO: For Round-Robin — track how many ticks each core has run
    //   its current process (for quantum preemption)
    // std::map<int, uint32_t> coreTicksUsed;

    // ── Synchronization ──────────────────────────────────────────────────
    mutable std::mutex       queueMutex;
    mutable std::mutex       listMutex;
    mutable std::mutex       coreMutex;
    std::condition_variable  schedulerCV;
    std::condition_variable  coreCV;

    // ── Batch generation ─────────────────────────────────────────────────
    int processCounter = 0; // for naming: p01, p02, ...
};
