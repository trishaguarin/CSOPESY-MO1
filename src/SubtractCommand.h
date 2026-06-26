// ============================================================================
// SubtractCommand.h — SUBTRACT Instruction
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Design the remaining commands:
//   ADD, SUBTRACT, SLEEP, FOR." (page 52 activity)
//
// MO1 REQUIREMENT: SUBTRACT(var1, var2/value, var3/value) (page 2)
//   - "Performs a subtraction operation: var1 = var2/value - var3/value"
//   - Same variable resolution rules as ADD
//   - Result clamped to [0, 65535]
//
// TODO: Implement execute()
//   - Resolve operand1 and operand2 (same as AddCommand)
//   - Compute: result = op1 - op2
//   - Clamp result to [0, 65535] (underflow → 0)
//   - Store in SymbolTable
// ============================================================================
#pragma once

#include "ICommand.h"
#include <string>

class SubtractCommand : public ICommand
{
public:
    // dest = operand1 - operand2
    SubtractCommand(const std::string& dest,
                    const std::string& operand1,
                    const std::string& operand2)
        : ICommand(CommandType::SUBTRACT),
          dest(dest), operand1(operand1), operand2(operand2) {}

    // TODO: Implement — resolve operands, subtract, clamp, store
    void execute(Process* process) override;

private:
    std::string dest;
    std::string operand1;
    std::string operand2;
};
