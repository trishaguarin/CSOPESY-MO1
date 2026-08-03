#pragma once

#include "ICommand.h"
#include "SymbolTable.h"

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <ctime>
#include <memory>
#include <cstdint>
#include <sstream>
#include <iomanip>

class IMemoryAllocator; // forward declaration

class Process
{
public:
    enum ProcessState
    {
        READY,
        RUNNING,
        WAITING,    // used by SLEEP instruction
        FINISHED,
        TERMINATED  // shut down due to memory access violation
    };

    Process(int pid, const std::string& name, size_t memorySize = 0);
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
        size_t       memorySize;
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
    size_t             getMemorySize()      const;

    // ── SymbolTable Access ───────────────────────────────────────────────
    SymbolTable& getSymbolTable();

    // ── Memory Allocator Access (set by Scheduler) ──────────────────────
    void setMemoryAllocator(IMemoryAllocator* alloc);
    IMemoryAllocator* getMemoryAllocator() const;

    // ── Setters ──────────────────────────────────────────────────────────
    void setState(ProcessState s);
    void setAssignedCore(int core);

    // ── Memory Access Violation ──────────────────────────────────────────
    void terminateWithViolation(uint32_t address);
    bool isTerminated() const;
    std::string getViolationAddress() const;
    std::string getViolationTime() const;

    // ── Sleep Management ─────────────────────────────────────────────────
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
    size_t                   memorySize;

    std::vector<std::shared_ptr<ICommand>> commandList;
    int commandCounter = 0;

    SymbolTable symbolTable;
    int sleepTicksRemaining = 0;

    std::vector<std::string> outputLog;

    // Memory allocator pointer (non-owning, set by Scheduler)
    IMemoryAllocator* memAllocator = nullptr;

    // Memory access violation info
    std::string violationAddr;
    std::string violationTimeStr;

    mutable std::mutex processMutex; // thread safety (mutable: lockable in const methods)
};
