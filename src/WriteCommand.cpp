#include "WriteCommand.h"
#include "Process.h"
#include "IMemoryAllocator.h"
#include <cstdlib>

void WriteCommand::execute(Process* process)
{
    auto* allocator = process->getMemoryAllocator();
    if (!allocator)
    {
        process->terminateWithViolation(address);
        return;
    }

    // Resolve value — try as literal integer first, then as variable name
    uint16_t value = 0;
    try
    {
        int parsed = std::stoi(valueSource);
        if (parsed < 0) parsed = 0;
        if (parsed > 65535) parsed = 65535;
        value = static_cast<uint16_t>(parsed);
    }
    catch (...)
    {
        // It's a variable name
        value = static_cast<uint16_t>(process->getSymbolTable().getVariable(valueSource));
    }

    bool ok = allocator->writeMemory(process->getName(), address, value);
    if (!ok)
    {
        process->terminateWithViolation(address);
        return;
    }
}
