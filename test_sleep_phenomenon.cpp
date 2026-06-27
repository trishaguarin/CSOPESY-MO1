// ============================================================================
// test_sleep_phenomenon.cpp — Verifies the Sleep Visibility Phenomenon
// ============================================================================
// PURPOSE:
//   Demonstrates WHY sleeping processes dominate screen -ls snapshots when
//   SleepCommand is mixed into the process instruction list.
//
// THEORY:
//   With quantum=1 (1 instruction per turn), a RUNNING process is visible
//   as "running" for ~1 tick. A sleeping process is visible as "sleeping"
//   for (sleep_duration) ticks. If avg sleep = 128 ticks, sleeping processes
//   appear 128x more frequently in any snapshot.
//
// TEST CASES:
//   Case A: 0%   sleep chance  → expect ~100% running in snapshots
//   Case B: 25%  sleep chance  → expect ~97%  sleeping in snapshots
//   Case C: 50%  sleep chance  → expect ~99%  sleeping in snapshots
//
// BUILD:
//   g++ -std=c++17 -o test_sleep.exe test_sleep_phenomenon.cpp
//     src/Process.cpp src/Scheduler.cpp src/ConfigParser.cpp
//     src/Console.cpp src/ScreenManager.cpp src/ReportGenerator.cpp
//     src/PrintCommand.cpp src/DeclareCommand.cpp src/AddCommand.cpp
//     src/SubtractCommand.cpp src/SleepCommand.cpp src/ForCommand.cpp
//     -lpthread
//
// RUN:
//   .\test_sleep.exe
// ============================================================================

#include "src/Scheduler.h"
#include "src/Process.h"
#include "src/PrintCommand.h"
#include "src/SleepCommand.h"
#include "src/DeclareCommand.h"
#include "src/AddCommand.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <random>

// ── Config builder ───────────────────────────────────────────────────────────

SystemConfig makeConfig(int numCpu, std::string algo, uint32_t quantum,
                        uint32_t batchFreq, uint32_t minIns, uint32_t maxIns)
{
    SystemConfig cfg;
    cfg.numCpu           = numCpu;
    cfg.schedulerAlgo    = algo;
    cfg.quantumCycles    = quantum;
    cfg.batchProcessFreq = batchFreq;
    cfg.minIns           = minIns;
    cfg.maxIns           = maxIns;
    cfg.delaysPerExec    = 0;
    return cfg;
}

// ── Process builder — controlled sleep probability ───────────────────────────

std::shared_ptr<Process> buildProcess(int pid, const std::string& name,
                                      int numInstructions, float sleepChance)
{
    static std::mt19937 rng(42); // fixed seed for reproducibility
    std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
    std::uniform_int_distribution<int>    sleepDist(1, 255);

    auto proc = std::make_shared<Process>(pid, name);

    for (int i = 0; i < numInstructions; ++i)
    {
        if (chanceDist(rng) < sleepChance)
            proc->addCommand(std::make_shared<SleepCommand>(sleepDist(rng)));
        else
            proc->addCommand(std::make_shared<PrintCommand>());
    }

    return proc;
}

// ── Snapshot taker ───────────────────────────────────────────────────────────

struct Snapshot
{
    int totalActive;   // RUNNING + WAITING
    int numRunning;    // state == RUNNING
    int numSleeping;   // state == WAITING
};

Snapshot takeSnapshot(Scheduler& sched)
{
    auto running  = sched.getRunningProcesses(); // includes RUNNING + WAITING
    Snapshot s = {0, 0, 0};
    s.totalActive = static_cast<int>(running.size());
    for (auto& p : running)
    {
        if (p->getState() == Process::RUNNING)  s.numRunning++;
        if (p->getState() == Process::WAITING)  s.numSleeping++;
    }
    return s;
}

// ── Test runner ──────────────────────────────────────────────────────────────

void runTest(const std::string& label,
             float sleepChance,
             int numProcesses,
             int instructionsPerProcess,
             int snapshotCount,
             int snapshotIntervalMs)
{
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  TEST: " << label << "\n";
    std::cout << "  Sleep chance per instruction: "
              << std::fixed << std::setprecision(0) << (sleepChance * 100) << "%\n";
    std::cout << "  Processes: " << numProcesses
              << "   Instructions each: " << instructionsPerProcess << "\n";
    std::cout << "  Config: RR, quantum=1, 4 cores\n";
    std::cout << "============================================================\n";

    // Build scheduler
    auto cfg = makeConfig(4, "rr", 1, 1, instructionsPerProcess, instructionsPerProcess);
    Scheduler sched(cfg);
    sched.start();

    // Add processes with controlled sleep probability
    for (int i = 1; i <= numProcesses; ++i)
    {
        std::string name = "p" + std::to_string(i);
        auto proc = buildProcess(i, name, instructionsPerProcess, sleepChance);
        sched.addProcess(proc);
    }

    // Print header
    std::cout << std::left
              << std::setw(8)  << "Snap#"
              << std::setw(10) << "Active"
              << std::setw(12) << "Running"
              << std::setw(12) << "Sleeping"
              << std::setw(14) << "Sleep %"
              << std::setw(12) << "CPU Util%"
              << "\n";
    std::cout << std::string(68, '-') << "\n";

    // Take snapshots
    int totalRunning  = 0;
    int totalSleeping = 0;
    int totalActive   = 0;

    for (int i = 1; i <= snapshotCount; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(snapshotIntervalMs));

        auto s   = takeSnapshot(sched);
        float pct = (s.totalActive > 0)
            ? (static_cast<float>(s.numSleeping) / s.totalActive) * 100.0f
            : 0.0f;

        totalRunning  += s.numRunning;
        totalSleeping += s.numSleeping;
        totalActive   += s.totalActive;

        std::cout << std::left
                  << std::setw(8)  << i
                  << std::setw(10) << s.totalActive
                  << std::setw(12) << s.numRunning
                  << std::setw(12) << s.numSleeping
                  << std::fixed << std::setprecision(1)
                  << std::setw(14) << pct
                  << std::setw(12) << sched.getCpuUtilization()
                  << "\n";
    }

    sched.stop();

    // Summary
    float avgSleepPct = (totalActive > 0)
        ? (static_cast<float>(totalSleeping) / totalActive) * 100.0f
        : 0.0f;

    std::cout << std::string(68, '-') << "\n";
    std::cout << "CONCLUSION: On average across " << snapshotCount
              << " snapshots, " << std::fixed << std::setprecision(1)
              << avgSleepPct << "% of visible active processes were SLEEPING.\n";

    if (sleepChance > 0.0f)
    {
        float expectedAvgSleep  = (1.0f + 255.0f) / 2.0f;  // avg sleep duration
        float expectedSleepPct  = (sleepChance * expectedAvgSleep)
                                / (sleepChance * expectedAvgSleep + (1.0f - sleepChance) * 1.0f)
                                * 100.0f;
        std::cout << "THEORY predicts: ~" << std::fixed << std::setprecision(1)
                  << expectedSleepPct << "% sleeping in any snapshot\n";
        std::cout << "  (sleep_chance * avg_sleep_ticks) / "
                     "(sleep_chance * avg_sleep_ticks + run_chance * 1 tick)\n";
    }
}

// ── Main ─────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║      CSOPESY — Sleep Visibility Phenomenon Test          ║\n";
    std::cout << "║                                                          ║\n";
    std::cout << "║  Demonstrates why sleeping processes dominate screen -ls ║\n";
    std::cout << "║  when SleepCommand is included in process instructions.  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    // ── Case A: No sleep — baseline ──────────────────────────────────────
    runTest(
        "Case A: 0% sleep (baseline)",
        0.0f,   // sleepChance
        20,     // numProcesses
        100,    // instructionsPerProcess
        8,      // snapshotCount
        300     // snapshotIntervalMs
    );

    // ── Case B: 25% sleep — matches original generator (case 3) ─────────
    runTest(
        "Case B: 25% sleep (original typeDist 0-3, case 3 = sleep)",
        0.25f,  // sleepChance
        20,     // numProcesses
        100,    // instructionsPerProcess
        8,      // snapshotCount
        300     // snapshotIntervalMs
    );

    // ── Case C: 50% sleep — high sleep frequency ─────────────────────────
    runTest(
        "Case C: 50% sleep (heavy sleep load)",
        0.50f,  // sleepChance
        20,     // numProcesses
        100,    // instructionsPerProcess
        8,      // snapshotCount
        300     // snapshotIntervalMs
    );

    std::cout << "\nDone. Key takeaway:\n";
    std::cout << "  With quantum=1 and avg SLEEP=128 ticks, each sleeping process\n";
    std::cout << "  is visible in snapshots ~128x longer than a running one.\n";
    std::cout << "  So even a 25% sleep chance dominates the screen -ls display.\n\n";

    return 0;
}
