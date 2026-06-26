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
    commandList.push_back(command);
}

void Process::executeCurrentCommand(int coreId)
{
    std::lock_guard<std::mutex> lock(processMutex);

    if (commandCounter >= static_cast<int>(commandList.size()))
    {
        state.store(FINISHED);
        return;
    }

    assignedCore.store(coreId);
    auto& cmd = commandList[commandCounter];
    cmd->execute(this);
}

void Process::moveToNextLine()
{
    std::lock_guard<std::mutex> lock(processMutex);

    commandCounter++;
    if (commandCounter >= static_cast<int>(commandList.size()))
    {
        state.store(FINISHED);
    }
}

// ── Getters ──────────────────────────────────────────────────────────────────
int                   Process::getPID()            const { return pid; }
std::string           Process::getName()           const { return name; }
Process::ProcessState Process::getState()          const { return state.load(); }
int                   Process::getAssignedCore()   const { return assignedCore.load(); }
int                   Process::getCommandCounter() const { return commandCounter; }
int                   Process::getTotalCommands()  const { return static_cast<int>(commandList.size()); }
std::time_t           Process::getCreationTime()   const { return creationTime; }
bool                  Process::isFinished()        const { return state.load() == FINISHED; }

// ── SymbolTable Access ───────────────────────────────────────────────────────
SymbolTable& Process::getSymbolTable()
{
    return symbolTable;
}

// ── Setters ──────────────────────────────────────────────────────────────────
void Process::setState(ProcessState s)    { state.store(s); }
void Process::setAssignedCore(int core)   { assignedCore.store(core); }

// ── Sleep Management ─────────────────────────────────────────────────────────

void Process::setSleepTicks(int ticks)
{
    sleepTicksRemaining = ticks;
    state.store(WAITING);
}

void Process::tickSleep()
{
    if (sleepTicksRemaining > 0)
    {
        sleepTicksRemaining--;
        if (sleepTicksRemaining == 0)
        {
            state.store(READY);
        }
    }
}

// ── Output Log ───────────────────────────────────────────────────────────────

void Process::appendToLog(const std::string& entry)
{
    // NOTE: processMutex is already held by executeCurrentCommand
    // so we don't re-lock here to avoid deadlock
    outputLog.push_back(entry);
}

const std::vector<std::string>& Process::getOutputLog() const
{
    return outputLog;
}

// ── Timestamp ────────────────────────────────────────────────────────────────
std::string Process::getTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_info;
#ifdef _WIN32
    localtime_s(&tm_info, &t);
#else
    localtime_r(&t, &tm_info);
#endif
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

std::string Process::getCreationTimestamp() const
{
    std::tm tm_info;
#ifdef _WIN32
    localtime_s(&tm_info, &creationTime);
#else
    localtime_r(&creationTime, &tm_info);
#endif
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
