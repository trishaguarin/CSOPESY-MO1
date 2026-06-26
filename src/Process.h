// ============================================================================
// Process.h — Process Control Block (PCB)
// ============================================================================
// LESSON REFERENCE: Midterm Review — "C++ Representation of a Process"
//   "In our emulator, we represent the PCB using a Process class. This
//    class manages the process lifecycle, its instructions, and its
//    local variables."
//
//   The PCB contains: Process State, Program Counter (commandCounter),
//   CPU Registers, CPU-Scheduling Information, Memory-Management Info,
//   Accounting Information, I/O Status Information.
//
// MO1 REQUIREMENT: Process with instruction execution
//   Each process has:
//   - A unique PID and human-readable name
//   - A list of ICommands to execute (commandList)
//   - A state (READY, RUNNING, WAITING, FINISHED)
//   - An assigned core (-1 if not assigned)
//   - A SymbolTable for variable storage
//   - Creation timestamp
//
// MO1 REQUIREMENT: SLEEP instruction
//   "Sleeps the current process for X CPU ticks and relinquishes the CPU."
//   → The process state changes to WAITING when sleeping
//
// MO1 REQUIREMENT: Variables
//   "Variables are stored in memory and will not be released until
//    the process finishes."
//   "uint16 variables are clamped between (0, max(uint16))."
//   "Variables are automatically declared with a value of 0 if they
//    have not yet been declared beforehand."
//
// LESSON REFERENCE: Process states (Midterm Review page 59)
//   - READY:    Process is waiting to be assigned to a processor
//   - RUNNING:  Instructions are being executed
//   - WAITING:  Process is waiting for some event (e.g., SLEEP)
//   - FINISHED: Process has finished execution (analogous to TERMINATED)
//
// REFERENCE: fcfs-scheduler branch (past activity)
//   Reusable: PID, name, state enum, core assignment, timestamps
//   Must add: ICommand list, SymbolTable, command counter
// ============================================================================
#pragma once

#include "ICommand.h"
#include "SymbolTable.h"

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <ctime>
#include <memory>

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

    Process(int pid, const std::string& name);
    ~Process();

    // ── Command Management ───────────────────────────────────────────────
    // LESSON REFERENCE: Midterm Review Process.h
    //   "void addCommand(std::shared_ptr<ICommand> command);"
    //   "void executeCurrentCommand();"
    //   "void moveToNextLine();"

    // TODO: Append a command to the process's command list
    //   Called during process creation to build the instruction sequence
    void addCommand(std::shared_ptr<ICommand> command);

    // TODO: Execute the command at the current commandCounter position
    //   - Get commandList[commandCounter]
    //   - Call command->execute(this)
    //   - Handle state changes (e.g., SLEEP sets state to WAITING)
    void executeCurrentCommand(int coreId);

    // TODO: Advance the command counter to the next instruction
    //   - commandCounter++
    //   - If commandCounter >= commandList.size(), set state to FINISHED
    void moveToNextLine();

    // ── Getters ──────────────────────────────────────────────────────────
    int                getPID()             const;
    std::string        getName()            const;
    ProcessState       getState()           const;
    int                getAssignedCore()    const;
    int                getCommandCounter()  const; // current instruction index
    int                getTotalCommands()   const; // total command count
    std::time_t        getCreationTime()    const;
    bool               isFinished()         const;

    // ── SymbolTable Access ───────────────────────────────────────────────
    // LESSON REFERENCE: Midterm Review — "SymbolTable& getSymbolTable();"
    //   Commands call this to read/write process-local variables
    SymbolTable& getSymbolTable();

    // ── Setters ──────────────────────────────────────────────────────────
    void setState(ProcessState s);
    void setAssignedCore(int core);

    // ── Sleep Management ─────────────────────────────────────────────────
    // TODO: Called by SleepCommand to initiate sleep
    void setSleepTicks(int ticks);

    // TODO: Check and decrement sleep counter (called each CPU tick)
    //   if (sleepTicksRemaining > 0) {
    //       sleepTicksRemaining--;
    //       if (sleepTicksRemaining == 0) {
    //           state = READY; // wake up, go back to ready queue
    //       }
    //   }
    void tickSleep();

    // ── Output Log ───────────────────────────────────────────────────────
    // TODO: Append to the output log (called by PrintCommand)
    void appendToLog(const std::string& entry);

    // TODO: Get the output log for process-smi display
    //   Returns the accumulated PRINT outputs for this process
    const std::vector<std::string>& getOutputLog() const;

    // ── Timestamp helper (reused from fcfs-scheduler branch) ─────────────
    static std::string getTimestamp();

private:
    int                      pid;
    std::string              name;
    std::atomic<ProcessState> state;
    std::atomic<int>         assignedCore;
    std::time_t              creationTime;

    // LESSON REFERENCE: Midterm Review Process.h
    //   "std::vector<std::shared_ptr<ICommand>> commandList;"
    //   "int commandCounter;"
    std::vector<std::shared_ptr<ICommand>> commandList;
    int commandCounter = 0;

    // LESSON REFERENCE: Midterm Review — SymbolTable
    //   "SymbolTable symbolTable;"
    SymbolTable symbolTable;

    // TODO: Sleep tracking — number of CPU ticks remaining
    int sleepTicksRemaining = 0;

    // TODO: Output log buffer (for process-smi display)
    //   PrintCommand appends entries here
    std::vector<std::string> outputLog;

    std::mutex processMutex; // thread safety
};
