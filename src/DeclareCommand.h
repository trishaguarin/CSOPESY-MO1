// ============================================================================
// DeclareCommand.h — DECLARE Instruction
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Rudimentary example of a variable
//   declaration command" (page 52)
//
// MO1 REQUIREMENT: DECLARE(var, value) (page 2)
//   - "Declares a uint16 with variable name 'var', and a default 'value'."
//   - Values clamped to [0, 65535]
//
// TODO: Implement execute()
//   - Call process->getSymbolTable().setVariable(varName, value)
//   - Value should be clamped to uint16 range [0, 65535]
// ============================================================================
#pragma once

#include "ICommand.h"
#include <string>
#include <cstdint>

class DeclareCommand : public ICommand
{
public:
    DeclareCommand(const std::string& varName, uint16_t value = 0)
        : ICommand(CommandType::DECLARE), varName(varName), value(value) {}

    // TODO: Implement — declare variable in process SymbolTable
    void execute(Process* process) override;

private:
    std::string varName;
    uint16_t    value;
};
