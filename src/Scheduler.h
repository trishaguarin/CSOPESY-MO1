// ============================================================================
// Scheduler.h — CPU Scheduler (FCFS & Round-Robin)
// ============================================================================
// LESSON REFERENCE: Midterm Review — "CPU Scheduling"
//   "CPU scheduling is a subset of process scheduling."
//   "CPU scheduling specifically deals with deciding which process to
//    execute next on the central processing unit (CPU)."
//
// LESSON REFERENCE: Midterm Review — "Emulating a CPU scheduler"
//   "Modify the scheduler such that the CPU core is actually a thread
//    worker. If there are four cores, then processes get assigned to
//    a 'core', where its commands are executed."
//
// LESSON REFERENCE: Midterm Review — Design #1
//   "There is only one ready queue." (shared ready queue model)
//
// MO1 REQUIREMENT: The scheduler (page 4-5)
//   "Your CPU scheduler is real-time and will continuously schedule
//    processes as long as your console is alive."
//   "The scheduler algorithm will be set through the 'initialize'
//    command and through the config.txt file."
//
// MO1 REQUIREMENT: CPU ticks (page 5)
//   "Assume that the CPU tick is an integer counter that tallies the
//    number of frame passes."
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
// MO1 REQUIREMENT: delay-per-exec (page 5)
//   "Delay before executing the next instruction in CPU cycles.
//    The delay is a 'busy-waiting' scheme wherein the process remains
//    in the CPU."
//
// REFERENCE: fcfs-scheduler branch (past activity)
//   Reusable: worker thread model, ready queue, core assignment logic
//   Must add: RR preemption, CPU tick model, batch generation,
//             delay-per-exec, config-driven parameters
// ============================================================================
#pragma once

#include "Process.h"
#include "ConfigParser.h"

#include <queue>
#include <vector>
#include <map>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

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
    std::shared_ptr<Process> findProcess(const std::string& name) const;

    int  getNumCores()       const;
    int  getCoresUsed()      const;
    int  getCoresAvailable() const;

    // CPU utilization calculation
    //   LESSON REFERENCE: Midterm Review activity (page 68)
    //     "Provide a function that can be called anytime from the main
    //      thread that writes down the CPU utilization."
    //   Tracks cumulative busy ticks / total ticks across all cores
    float getCpuUtilization() const;

    uint64_t getCpuTicks() const;

    const SystemConfig& getConfig() const { return config; }

private:
    // ── Scheduler Thread ─────────────────────────────────────────────────
    // Implements the main scheduling loop:
    //   while (running) {
    //       cpuTickCounter++;
    //       // batch generation every batchProcessFreq ticks
    //       // handle sleeping processes (tick WAITING → READY)
    //       // assign ready processes to idle cores
    //       // RR: preempt cores that exceeded quantum
    //   }
    //
    // LESSON REFERENCE: Midterm Review — "Algorithmic Overview of Round-Robin"
    //   "For each CPU cycle, do the following:
    //    a. Update R if there's any pending process to be scheduled.
    //    b. Select the first process in R to be the candidate.
    //    c. Execute candidate. candidate.C++;
    //    d. If candidate.C == T, then perform #b. Put candidate at end of R."
    void schedulerLoop();

    // ── Core Worker Threads ──────────────────────────────────────────────
    // LESSON REFERENCE: Midterm Review activity (page 67-68)
    //   "CPU core is actually a thread worker. If there are four cores,
    //    then processes get assigned to a 'core', where its commands
    //    are executed."
    //
    // Each core worker:
    //   1. Waits for a process assignment (via condition variable)
    //   2. Calls process->executeCurrentCommand(coreId) per tick
    //   3. Calls process->moveToNextLine() after each execution
    //   4. Between commands, busy-waits for delay-per-exec ticks
    //   5. For RR: yields after quantum-cycles ticks
    //   6. When process finishes, is preempted, or SLEEPs, signals scheduler
    void coreWorker(int coreId);

    // ── Batch Process Generator ──────────────────────────────────────────
    // Generates a new process with:
    //   - Human-readable name (p01, p02, ..., p1240)
    //   - Random command count between min-ins and max-ins (from config)
    //   - PrintCommand objects (default "Hello world from <name>!")
    std::shared_ptr<Process> generateProcess();

    // ── Configuration ────────────────────────────────────────────────────
    SystemConfig config;

    // ── State ────────────────────────────────────────────────────────────
    std::atomic<bool>     running{false};
    std::atomic<bool>     batchGenerating{false};
    std::atomic<uint64_t> cpuTickCounter{0};

    // ── Process Queues ───────────────────────────────────────────────────
    // LESSON REFERENCE: Design #1 — "There is only one ready queue."
    std::queue<std::shared_ptr<Process>>   readyQueue;
    std::vector<std::shared_ptr<Process>>  allProcesses;

    // ── Threading ────────────────────────────────────────────────────────
    std::thread              schedulerThread;
    std::vector<std::thread> coreThreads;

    // ── Per-Core State ───────────────────────────────────────────────────
    std::vector<bool>                              coreStatus;   // true = busy
    std::map<int, std::shared_ptr<Process>>        coreProcess;  // coreId → process

    // For Round-Robin — track how many ticks each core has run
    //   its current process (for quantum preemption)
    // LESSON REFERENCE: Midterm Review — Round-Robin
    //   "If candidate.C == T, then put candidate at the end of R."
    std::map<int, uint32_t> coreTicksUsed;

    // ── Synchronization ──────────────────────────────────────────────────
    mutable std::mutex       queueMutex;
    mutable std::mutex       listMutex;
    mutable std::mutex       coreMutex;
    std::condition_variable  schedulerCV;
    std::condition_variable  coreCV;

    // ── Batch generation ─────────────────────────────────────────────────
    int processCounter = 0; // for naming: p01, p02, ...

    // ── CPU utilization tracking ─────────────────────────────────────────
    std::atomic<uint64_t> totalBusyTicks{0};
    std::atomic<uint64_t> totalIdleTicks{0};
};
