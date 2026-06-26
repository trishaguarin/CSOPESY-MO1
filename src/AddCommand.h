// ============================================================================
// AddCommand.h — ADD Instruction
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Design the remaining commands:
//   ADD, SUBTRACT, SLEEP, FOR." (page 52 activity)
//
// MO1 REQUIREMENT: ADD(var1, var2/value, var3/value) (page 2)
//   - "Performs an addition operation: var1 = var2/value + var3/value"
//   - "var1, var2, var3 are variables. Variables are automatically
//      declared with a value of 0 if they have not yet been declared
//      beforehand. Can also add a uint16 value."
//   - Result clamped to [0, 65535]
//
// TODO: Implement execute()
//   - Resolve operand1 and operand2:
//     - If operand is a variable name, get from SymbolTable (auto-declare 0)
//     - If operand is a numeric literal, parse to uint16
//   - Compute: result = op1 + op2
//   - Clamp result to [0, 65535]
//   - Store in SymbolTable: process->getSymbolTable().setVariable(dest, result)
// ============================================================================
#pragma once

#include "ICommand.h"
#include <string>

class AddCommand : public ICommand
{
public:
    // dest = operand1 + operand2
    // Operands can be variable names (resolved from SymbolTable) or literals
    AddCommand(const std::string& dest,
               const std::string& operand1,
               const std::string& operand2)
        : ICommand(CommandType::ADD),
          dest(dest), operand1(operand1), operand2(operand2) {}

    // TODO: Implement — resolve operands, add, clamp, store
    void execute(Process* process) override;

private:
    std::string dest;     // target variable name
    std::string operand1; // variable name or literal
    std::string operand2; // variable name or literal
};
