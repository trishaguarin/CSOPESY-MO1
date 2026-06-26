// ============================================================================
// PrintCommand.h — PRINT Instruction
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Representing process instructions"
//   The review shows a PrintCommand class that inherits from ICommand
//   and writes output to the process's log.
//
// MO1 REQUIREMENT: PRINT(msg) (page 2)
//   - "Display an output 'msg' to the console. The output can only be
//      seen when the user is inside its attached screen."
//   - "The 'msg' can print 1 variable. E.g. PRINT('Value from: ' + x)"
//   - "Unless specified in the test case, the 'msg' in the PRINT function
//      should always be 'Hello world from <process_name>!'"
//
// TODO: Implement execute()
//   - Construct the output string (default: "Hello world from <name>!")
//   - If msg contains a variable reference, resolve from SymbolTable
//   - Append formatted output to the process's output log
//   - Format: "<timestamp> Core:<coreId> <msg>"
// ============================================================================
#pragma once

#include "ICommand.h"
#include <string>

class PrintCommand : public ICommand
{
public:
    PrintCommand(const std::string& message = "")
        : ICommand(CommandType::PRINT), message(message) {}

    // TODO: Implement — write message to process output log
    void execute(Process* process) override;

private:
    std::string message; // if empty, use default "Hello world from <name>!"
};
