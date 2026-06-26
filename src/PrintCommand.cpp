#include "PrintCommand.h"
#include "Process.h"

void PrintCommand::execute(Process* process)
{
    std::string output;
    if (message.empty())
    {
        output = "Hello world from " + process->getName() + "!";
    }
    else
    {
        output = message;
    }

    std::string logEntry = Process::getTimestamp()
        + " Core:" + std::to_string(process->getAssignedCore())
        + " \"" + output + "\"";

    process->appendToLog(logEntry);
}
