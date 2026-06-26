#include "SleepCommand.h"
#include "Process.h"

void SleepCommand::execute(Process* process)
{
    process->setSleepTicks(static_cast<int>(ticks));
    // Process state is set to WAITING inside setSleepTicks
}
