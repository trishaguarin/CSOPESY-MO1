// ============================================================================
// SymbolTable.h — Process Variable Storage
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Symbol Table"
//   "A process needs to store and retrieve variables during execution.
//    The Symbol Table maps variable names (strings) to their values."
//
//   "The OS emulator explicitly states that uint16 variables are stored
//    in the symbol table segment of the process."
//
// MO1 REQUIREMENT: Variables (page 2-3)
//   - "Variables are stored in memory and will not be released until
//      the process finishes."
//   - "uint16 variables are clamped between (0, max(uint16))."
//   - "Variables are automatically declared with a value of 0 if they
//      have not yet been declared beforehand."
//
// DESIGN: Follows the review's SymbolTable class with unordered_map
//   - setVariable(): stores or updates a variable
//   - getVariable(): returns value, or 0 if not declared (auto-declare)
//   - hasVariable(): checks existence without side effects
// ============================================================================
#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

class SymbolTable
{
public:
    // TODO: Store or update a variable with uint16 clamping [0, 65535]
    void setVariable(const std::string& name, int value);

    // TODO: Retrieve a variable's value
    //   - If not found, auto-declare with value 0 (per MO1 spec)
    //   - "Variables are automatically declared with a value of 0 if they
    //      have not yet been declared beforehand."
    int getVariable(const std::string& name);

    // TODO: Check if a variable exists without modifying state
    bool hasVariable(const std::string& name) const;

private:
    std::unordered_map<std::string, int> table;
};
