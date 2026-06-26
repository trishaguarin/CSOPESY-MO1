// ============================================================================
// Process.cpp — Process Control Block Implementation
// ============================================================================
// LESSON REFERENCE: Midterm Review — "C++ Representation of a Process"
//   Implements the PCB lifecycle: command execution, variable storage,
//   sleep management, and output logging.
//
// REFERENCE: fcfs-scheduler branch (past activity)
//   The old executePrint() only did:
//     outFile << timestamp << "Hello world from " << name << "!\n"
//     printsExecuted++; sleep_for(25ms)
//   The new version uses the ICommand pattern with CPU tick-based execution.
// ============================================================================

#include "Process.h"
#include <chrono>
#include <sstream>
#include <iomanip>

Process::Process(int pid, const std::string& name)
    : pid(pid), name(name),
      state(READY), assignedCore(-1), commandCounter(0),
      sleepTicksRemaining(0)
{
    auto now = std::chrono::system_clock::now();
    creationTime = std::chrono::system_clock::to_time_t(now);
}

Process::~Process() = default;

// ── Command Management ───────────────────────────────────────────────────────

void Process::addCommand(std::shared_ptr<ICommand> command)
{
    // TODO: Append command to the commandList
    //   commandList.push_back(command);
}

void Process::executeCurrentCommand(int coreId)
{
    std::lock_guard<std::mutex> lock(processMutex);

    // TODO: Execute the current command using the ICommand interface
    //
    // Pseudocode:
    //   if (commandCounter >= commandList.size()) {
    //       state = FINISHED;
    //       return;
    //   }
    //
    //   assignedCore = coreId;
    //   auto& cmd = commandList[commandCounter];
    //   cmd->execute(this);
    //
    //   // Note: moveToNextLine() is called separately by the core worker
    //   // unless the command itself needs special handling (e.g., SLEEP
    //   // sets state to WAITING before the counter advances, FOR loops
    //   // expand their body inline)
}

void Process::moveToNextLine()
{
    // TODO: Advance the command counter
    //   commandCounter++;
    //   if (commandCounter >= static_cast<int>(commandList.size())) {
    //       state = FINISHED;
    //   }
}

// ── Getters ──────────────────────────────────────────────────────────────────
int                Process::getPID()            const { return pid; }
std::string        Process::getName()           const { return name; }
Process::ProcessState Process::getState()       const { return state.load(); }
int                Process::getAssignedCore()   const { return assignedCore.load(); }
int                Process::getCommandCounter() const { return commandCounter; }
int                Process::getTotalCommands()  const { return static_cast<int>(commandList.size()); }
std::time_t        Process::getCreationTime()   const { return creationTime; }
bool               Process::isFinished()        const { return state.load() == FINISHED; }

// ── SymbolTable Access ───────────────────────────────────────────────────────
SymbolTable& Process::getSymbolTable()
{
    return symbolTable;
}

// ── Setters ──────────────────────────────────────────────────────────────────
void Process::setState(ProcessState s) { state.store(s); }
void Process::setAssignedCore(int core) { assignedCore.store(core); }

// ── Sleep Management ─────────────────────────────────────────────────────────

void Process::setSleepTicks(int ticks)
{
    // TODO: Set sleep duration and transition to WAITING
    //   sleepTicksRemaining = ticks;
    //   state = WAITING;
}

void Process::tickSleep()
{
    // TODO: Decrement sleepTicksRemaining each CPU tick
    //   if (sleepTicksRemaining > 0) {
    //       sleepTicksRemaining--;
    //       if (sleepTicksRemaining == 0) {
    //           state = READY; // wake up, return to ready queue
    //       }
    //   }
}

// ── Output Log ───────────────────────────────────────────────────────────────

void Process::appendToLog(const std::string& entry)
{
    // TODO: Append an entry to the output log
    //   std::lock_guard<std::mutex> lock(processMutex);
    //   outputLog.push_back(entry);
}

const std::vector<std::string>& Process::getOutputLog() const
{
    return outputLog;
}

// ── Timestamp (reused from fcfs-scheduler branch) ────────────────────────────
std::string Process::getTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_info;
    std::tm* tm_ptr = std::localtime(&t);
    if (tm_ptr)
    {
        tm_info = *tm_ptr;
    }
    else
    {
        std::memset(&tm_info, 0, sizeof(tm_info));
    }
    std::ostringstream oss;
    int hour = tm_info.tm_hour;
    const char* ampm = (hour >= 12) ? "PM" : "AM";
    if (hour == 0) hour = 12;
    else if (hour > 12) hour -= 12;

    oss << "("
        << std::setw(2) << std::setfill('0') << (tm_info.tm_mon + 1) << "/"
        << std::setw(2) << std::setfill('0') << tm_info.tm_mday << "/"
        << (tm_info.tm_year + 1900) << " "
        << std::setw(2) << std::setfill('0') << hour << ":"
        << std::setw(2) << std::setfill('0') << tm_info.tm_min << ":"
        << std::setw(2) << std::setfill('0') << tm_info.tm_sec
        << ampm << ")";
    return oss.str();
}
