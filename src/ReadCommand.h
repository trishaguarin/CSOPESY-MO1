#pragma once

#include "ICommand.h"
#include <string>
#include <cstdint>

// READ varName 0xADDR — reads uint16 from memory address into symbol table variable
class ReadCommand : public ICommand
{
public:
    ReadCommand(const std::string& varName, uint32_t address)
        : ICommand(MEM_READ), varName(varName), address(address) {}

    void execute(Process* process) override;

private:
    std::string varName;
    uint32_t    address;
};
