#include "DeclareCommand.h"
#include "Process.h"

void DeclareCommand::execute(Process* process)
{
    process->getSymbolTable().setVariable(varName, static_cast<int>(value));
}
