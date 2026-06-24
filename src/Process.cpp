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
    std::lock_guard<std::mutex> lock(processMutex);

    // TODO: Implement instruction execution
    //
    // Pseudocode:
    //   if (instructionPointer >= instructions.size()) {
    //       state = FINISHED;
    //       return false;
    //   }
    //
    //   auto& instr = instructions[instructionPointer];
    //   switch (instr.type) {
    //       case InstructionType::PRINT:
    //           // Append to outputLog: timestamp + core + msg
    //           // Default msg: "Hello world from <name>!"
    //           // If msg contains a variable reference, resolve from variableStore
    //           break;
    //
    //       case InstructionType::DECLARE:
    //           // variableStore[instr.varName] = instr.value;
    //           break;
    //
    //       case InstructionType::ADD:
    //           // Resolve operand1 and operand2 (variable name → value, or literal)
    //           // variableStore[instr.varName] = clamp(op1 + op2, 0, 65535);
    //           // Auto-declare if variable not found (default 0)
    //           break;
    //
    //       case InstructionType::SUBTRACT:
    //           // Same as ADD but subtraction
    //           // Clamp to [0, 65535]
    //           break;
    //
    //       case InstructionType::SLEEP:
    //           // sleepTicksRemaining = instr.sleepTicks;
    //           // state = WAITING;
    //           // return true; (process gives up CPU)
    //           break;
    //
    //       case InstructionType::FOR:
    //           // Push onto forStack: {current instructionPointer, 0, instr.repeatCount}
    //           // On each iteration, execute body instructions
    //           // When iterations complete, pop from forStack
    //           // Max nesting depth: 3
    //           break;
    //   }
    //
    //   instructionPointer++;
    //   return true;

    return false; // stub
}

// ── Sleep Tick ───────────────────────────────────────────────────────────────
void Process::tickSleep()
{
    // TODO: Decrement sleepTicksRemaining
    //   if (sleepTicksRemaining > 0) {
    //       sleepTicksRemaining--;
    //       if (sleepTicksRemaining == 0) {
    //           state = READY; // wake up, go back to ready queue
    //       }
    //   }
}

// ── Timestamp (reused from old Process.h) ────────────────────────────────────
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
