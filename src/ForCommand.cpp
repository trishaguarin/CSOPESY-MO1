#include "ForCommand.h"
#include "Process.h"

void ForCommand::execute(Process* process)
{
    for (int i = 0; i < repeatCount; ++i)
    {
        for (auto& cmd : body)
        {
            cmd->execute(process);
            // If the process entered WAITING (e.g., SLEEP inside FOR), stop
            if (process->getState() == Process::WAITING)
                return;
        }
    }
}
