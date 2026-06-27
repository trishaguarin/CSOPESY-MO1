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

    
    void addCommand(std::shared_ptr<ICommand> command);
    void executeCurrentCommand(int coreId);
    void moveToNextLine();

    // ── Display Snapshot (atomic read of state + core together) ──────────
    struct DisplaySnapshot
    {
        ProcessState state;
        int          assignedCore;
        int          commandCounter;
        int          totalCommands;
        std::string  name;
        std::string  creationTimestamp;
    };
    DisplaySnapshot getDisplaySnapshot() const;

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
    SymbolTable& getSymbolTable();

    // ── Setters ──────────────────────────────────────────────────────────
    void setState(ProcessState s);
    void setAssignedCore(int core);

    // ── Sleep Management ─────────────────────────────────────────────────
    // TODO: Called by SleepCommand to initiate sleep
    void setSleepTicks(int ticks);
    void tickSleep();

    // ── Output Log ───────────────────────────────────────────────────────
    void appendToLog(const std::string& entry);
    const std::vector<std::string>& getOutputLog() const;

    // ── Timestamp helpers ─────────────────────────────────────────────────
    static std::string getTimestamp();
    std::string getCreationTimestamp() const;

private:
    int                      pid;
    std::string              name;
    std::atomic<ProcessState> state;
    std::atomic<int>         assignedCore;
    std::time_t              creationTime;

    std::vector<std::shared_ptr<ICommand>> commandList;
    int commandCounter = 0;

    SymbolTable symbolTable;
    int sleepTicksRemaining = 0;

    std::vector<std::string> outputLog;

    mutable std::mutex processMutex; // thread safety (mutable: lockable in const methods)
};
