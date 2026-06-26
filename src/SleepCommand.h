// ============================================================================
// SleepCommand.h — SLEEP Instruction
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Design the remaining commands:
//   ADD, SUBTRACT, SLEEP, FOR." (page 52 activity)
//
// MO1 REQUIREMENT: SLEEP(X) (page 2)
//   - "Sleeps the current process for X (uint8) CPU ticks and
//      relinquishes the CPU."
//
// LESSON REFERENCE: Process states (Midterm Review page 59)
//   - When a process sleeps, it transitions from RUNNING → WAITING
//   - After X CPU ticks elapse, the process transitions WAITING → READY
//   - While WAITING, the process does NOT occupy a CPU core
//
// TODO: Implement execute()
//   - Set process's sleepTicksRemaining = ticks
//   - Set process state to WAITING
//   - The scheduler's tick loop will handle decrementing and waking up
// ============================================================================
#pragma once

#include "ICommand.h"
#include <cstdint>

class SleepCommand : public ICommand
{
public:
    SleepCommand(uint8_t ticks)
        : ICommand(CommandType::SLEEP), ticks(ticks) {}

    // TODO: Implement — set process to WAITING for 'ticks' CPU cycles
    void execute(Process* process) override;

private:
    uint8_t ticks; // number of CPU ticks to sleep
};
