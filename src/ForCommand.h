// ============================================================================
// ForCommand.h — FOR Loop Instruction
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Design the remaining commands:
//   ADD, SUBTRACT, SLEEP, FOR." (page 52 activity)
//
// MO1 REQUIREMENT: FOR([instructions], repeats) (page 2-3)
//   - "Performs a for-loop, given a set/array of instructions.
//      Can be nested."
//   - "For loops can be nested up to 3 times."
//
// DESIGN NOTES:
//   - The body is a vector of ICommand pointers (sub-commands)
//   - When executed, the body is repeated 'repeatCount' times
//   - Nesting is handled naturally since body commands can include
//     other ForCommands (up to 3 levels deep)
//   - The process's commandCounter management needs special handling:
//     FOR loops expand inline during execution
//
// TODO: Implement execute()
//   - For each iteration (0 to repeatCount-1):
//     - Execute each command in the body sequentially
//   - Track nesting depth to enforce the 3-level max
// ============================================================================
#pragma once

#include "ICommand.h"
#include <vector>
#include <memory>

class ForCommand : public ICommand
{
public:
    ForCommand(const std::vector<std::shared_ptr<ICommand>>& body, int repeatCount)
        : ICommand(CommandType::FOR), body(body), repeatCount(repeatCount) {}

    // TODO: Implement — execute body commands repeatCount times
    void execute(Process* process) override;

private:
    std::vector<std::shared_ptr<ICommand>> body; // nested commands
    int repeatCount;
};
