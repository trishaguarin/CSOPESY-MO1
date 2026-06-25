// ============================================================================
// Process.h — Process Representation
// ============================================================================
// MO1 REQUIREMENT: Process with instruction execution
//   Each process has:
//   - A unique PID and human-readable name
//   - A list of Instructions to execute
//   - A state (READY, RUNNING, WAITING, FINISHED)
//   - An assigned core (-1 if not assigned)
//   - A per-process variable store (map<string, uint16_t>)
//   - Creation timestamp
//
// MO1 REQUIREMENT: SLEEP instruction
//   "Sleeps the current process for X CPU ticks and relinquishes the CPU."
//   → The process state should change to WAITING when sleeping
//
// MO1 REQUIREMENT: Variables
//   "Variables are stored in memory and will not be released until
//    the process finishes."
//   "uint16 variables are clamped between (0, max(uint16))."
//   "Variables are automatically declared with a value of 0 if they
//    have not yet been declared beforehand."
//
// REFERENCE: FCFS-scheduler/Process.h
//   Reusable: PID, name, state enum, core assignment, timestamps
//   Must add:  instruction list, variable store, instruction pointer
// ============================================================================
#pragma once

#include "Instruction.h"

#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <ctime>
#include <fstream>

class Process
{
public:
    enum ProcessState
    {
        READY,
        RUNNING,
        WAITING,  // used by SLEEP instruction
        FINISHED
    };

    Process(int pid, const std::string& name,
            const std::vector<Instruction>& instructions);
    ~Process();

    // ── Getters ──────────────────────────────────────────────────────────
    int                getPID()             const;
    std::string        getName()            const;
    ProcessState       getState()           const;
    int                getAssignedCore()    const;
    int                getCurrentLine()     const; // instruction pointer
    int                getTotalLines()      const; // total instruction count
    std::time_t        getCreationTime()    const;
    bool               isFinished()         const;

    // ── Setters ──────────────────────────────────────────────────────────
    void setState(ProcessState s);
    void setAssignedCore(int core);

    // ── Execution ────────────────────────────────────────────────────────
    // TODO: Execute the next instruction in this process's instruction list
    //   - Advance the instruction pointer
    //   - Handle each InstructionType appropriately
    //   - For PRINT: write to the process's log / output buffer
    //   - For DECLARE: add to variableStore
    //   - For ADD/SUBTRACT: compute and store in variableStore
    //   - For SLEEP: set state to WAITING, record remaining sleep ticks
    //   - For FOR: manage loop counter and nested instruction pointer
    //   - Return true if instruction executed, false if process is finished
    bool executeNextInstruction(int coreId);

    // TODO: Check and decrement sleep counter (called each CPU tick)
    //   If sleepTicksRemaining > 0, decrement.
    //   If it reaches 0, set state back to READY.
    void tickSleep();

    // ── Logging ──────────────────────────────────────────────────────────
    // TODO: Get the output log for process-smi display
    //   Returns the accumulated PRINT outputs for this process
    std::string getOutputLog() const;

    // ── Timestamp helper (reused from old Process.h) ─────────────────────
    static std::string getTimestamp();

private:
    int                      pid;
    std::string              name;
    std::vector<Instruction> instructions;
    std::atomic<ProcessState> state;
    std::atomic<int>         assignedCore;
    std::time_t              creationTime;

    // TODO: Instruction pointer — tracks which instruction to execute next
    int instructionPointer = 0;

    // TODO: Variable store for DECLARE/ADD/SUBTRACT
    //   - Auto-declare with value 0 if not found
    //   - Clamp to [0, 65535]
    std::map<std::string, uint16_t> variableStore;

    // TODO: Sleep tracking
    int sleepTicksRemaining = 0;

    // TODO: FOR loop state tracking
    //   Need a stack of {loop start index, current iteration, max iterations}
    //   for nested FOR loops (up to 3 deep)
    // struct ForLoopState { int startIdx; int current; int total; };
    // std::vector<ForLoopState> forStack;

    // TODO: Output log buffer (for process-smi display)
    std::vector<std::string> outputLog;
};
