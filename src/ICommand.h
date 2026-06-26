// ============================================================================
// ICommand.h — Abstract Command Interface
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Representing process instructions"
//   The general idea is to create a command/instruction interface that all
//   other instructions will implement. E.g., print, add, read, write, etc.
//
//   Each command class implements execute() which operates on the Process's
//   SymbolTable for variable storage and state management.
//
// MO1 REQUIREMENT: Barebones process instructions (page 2-3)
//   PRINT(msg)                        — display output to console
//   DECLARE(var, value)               — declare uint16 variable
//   ADD(var1, var2/value, var3/value)  — var1 = var2 + var3
//   SUBTRACT(var1, var2/value, var3/value) — var1 = var2 - var3
//   SLEEP(X)                          — sleep for X CPU ticks, relinquish CPU
//   FOR([instructions], repeats)      — loop, nestable up to 3 times
//
// DESIGN PATTERN: Command Pattern (from Midterm Review)
//   - ICommand is the abstract interface
//   - Concrete commands: PrintCommand, DeclareCommand, AddCommand, etc.
//   - Process holds a std::vector<std::shared_ptr<ICommand>> commandList
//   - Each command receives a pointer to its parent Process so it can
//     access the SymbolTable and output log
// ============================================================================
#pragma once

#include <string>
#include <memory>

// Forward declaration — commands need access to Process for SymbolTable
class Process;

class ICommand
{
public:
    enum CommandType
    {
        PRINT,
        DECLARE,
        ADD,
        SUBTRACT,
        SLEEP,
        FOR
    };

    ICommand(CommandType type) : type(type) {}
    virtual ~ICommand() = default;

    // TODO: Execute this command on the given process
    //   - Each concrete command implements its own logic
    //   - Commands access process->getSymbolTable() for variables
    //   - PRINT commands append to the process's output log
    //   - SLEEP commands change the process state to WAITING
    virtual void execute(Process* process) = 0;

    CommandType getType() const { return type; }

protected:
    CommandType type;
};
