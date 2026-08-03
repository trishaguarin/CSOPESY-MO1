#include "ReadCommand.h"
#include "Process.h"
#include "IMemoryAllocator.h"

void ReadCommand::execute(Process* process)
{
    auto* allocator = process->getMemoryAllocator();
    if (!allocator)
    {
        process->terminateWithViolation(address);
        return;
    }

    uint16_t value = 0;
    bool ok = allocator->readMemory(process->getName(), address, value);
    if (!ok)
    {
        process->terminateWithViolation(address);
        return;
    }

    process->getSymbolTable().setVariable(varName, static_cast<int>(value));
}
