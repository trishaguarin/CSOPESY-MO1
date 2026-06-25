// ============================================================================
// Process.cpp — Process Implementation
// ============================================================================
// MO1 REQUIREMENT: Process execution with instruction support
//
// REFERENCE: FCFS-scheduler/Process.h
//   The old executePrint() only did:
//     outFile << timestamp << "Hello world from " << name << "!\n"
//     printsExecuted++; sleep_for(25ms)
//   The new version must support all instruction types and use CPU ticks.
// ============================================================================

#include "Process.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstring>

static uint16_t resolveOperand(const std::string& token,
                               const std::map<std::string, uint16_t>& store)
{
    if (token.empty())
        return 0;

    bool isNumber = true;
    for (char c : token)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            isNumber = false;
            break;
        }
    }

    if (isNumber)
    {
        unsigned long value = std::stoul(token);
        return static_cast<uint16_t>(value > 0xFFFFu ? 0xFFFFu : value);
    }

    auto it = store.find(token);
    if (it != store.end())
        return it->second;

    return 0;
}

Process::Process(int pid, const std::string& name,
                 const std::vector<Instruction>& instructions)
    : pid(pid), name(name), instructions(instructions),
      state(READY), assignedCore(-1), instructionPointer(0)
{
    auto now = std::chrono::system_clock::now();
    creationTime = std::chrono::system_clock::to_time_t(now);
}

Process::~Process() = default;

// ── Getters ──────────────────────────────────────────────────────────────────
int                Process::getPID()          const { return pid; }
std::string        Process::getName()         const { return name; }
Process::ProcessState Process::getState()     const { return state.load(); }
int                Process::getAssignedCore() const { return assignedCore.load(); }
int                Process::getCurrentLine()  const { return instructionPointer; }
int                Process::getTotalLines()   const { return static_cast<int>(instructions.size()); }
std::time_t        Process::getCreationTime() const { return creationTime; }
bool               Process::isFinished()      const { return state.load() == FINISHED; }

// ── Setters ──────────────────────────────────────────────────────────────────
void Process::setState(ProcessState s) { state.store(s); }
void Process::setAssignedCore(int core) { assignedCore.store(core); }

// ── Execution ────────────────────────────────────────────────────────────────
bool Process::executeNextInstruction(int coreId)
{
    if (state.load() == FINISHED)
        return false;

    if (instructionPointer >= static_cast<int>(instructions.size()))
    {
        state = FINISHED;
        return false;
    }

    const Instruction& instr = instructions[instructionPointer];
    switch (instr.type)
    {
        case InstructionType::PRINT:
        {
            std::string text = instr.msg.empty()
                ? "Hello world from " + name + "!"
                : instr.msg;

            std::ostringstream oss;
            oss << getTimestamp() << " [Core " << coreId << "] " << text;
            outputLog.push_back(oss.str());
            break;
        }

        case InstructionType::DECLARE:
            variableStore[instr.varName] = instr.value;
            break;

        case InstructionType::ADD:
        {
            uint16_t left = resolveOperand(instr.operand1, variableStore);
            uint16_t right = resolveOperand(instr.operand2, variableStore);
            uint32_t sum = static_cast<uint32_t>(left) + static_cast<uint32_t>(right);
            variableStore[instr.varName] = static_cast<uint16_t>(sum > 0xFFFFu ? 0xFFFFu : sum);
            break;
        }

        case InstructionType::SUBTRACT:
        {
            uint16_t left = resolveOperand(instr.operand1, variableStore);
            uint16_t right = resolveOperand(instr.operand2, variableStore);
            int32_t diff = static_cast<int32_t>(left) - static_cast<int32_t>(right);
            variableStore[instr.varName] = static_cast<uint16_t>(diff < 0 ? 0 : diff);
            break;
        }

        case InstructionType::SLEEP:
            sleepTicksRemaining = static_cast<int>(instr.sleepTicks);
            state = WAITING;
            instructionPointer++;
            return true;

        case InstructionType::FOR:
            // TODO: Implement FOR loop execution
            break;
    }

    instructionPointer++;
    if (instructionPointer >= static_cast<int>(instructions.size()))
        state = FINISHED;

    return true;
}

// ── Sleep Tick ───────────────────────────────────────────────────────────────
void Process::tickSleep()
{
    if (sleepTicksRemaining > 0)
    {
        sleepTicksRemaining--;
        if (sleepTicksRemaining == 0 && state.load() == WAITING)
        {
            state = READY;
        }
    }
}

std::string Process::getOutputLog() const
{
    std::ostringstream oss;
    for (const auto& line : outputLog)
    {
        oss << line << '\n';
    }
    return oss.str();
}

// ── Timestamp (reused from old Process.h) ────────────────────────────────────
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
