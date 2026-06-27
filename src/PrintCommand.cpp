#include "PrintCommand.h"
#include "Process.h"

void PrintCommand::execute(Process* process)
{
    std::string output;
    if (message.empty() && varName.empty())
    {
        output = "Hello world from " + process->getName() + "!";
    }
    else
    {
        output = message;
        if (!varName.empty())
        {
            int val = process->getSymbolTable().getVariable(varName);
            output += std::to_string(val);
        }
    }

    std::string logEntry = Process::getTimestamp()
        + " Core:" + std::to_string(process->getAssignedCore())
        + " \"" + output + "\"";

    process->appendToLog(logEntry);
}
