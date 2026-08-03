#pragma once

#include "ICommand.h"
#include <string>
#include <cstdint>

// WRITE 0xADDR value — writes uint16 value to memory address
class WriteCommand : public ICommand
{
public:
    // valueSource can be a literal number or a variable name
    WriteCommand(uint32_t address, const std::string& valueSource)
        : ICommand(MEM_WRITE), address(address), valueSource(valueSource) {}

    void execute(Process* process) override;

private:
    uint32_t    address;
    std::string valueSource; // literal int or variable name
};
