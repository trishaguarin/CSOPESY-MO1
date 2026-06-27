#pragma once

#include "ICommand.h"
#include <string>
#include <cstdint>

class DeclareCommand : public ICommand
{
public:
    DeclareCommand(const std::string& varName, uint16_t value = 0)
        : ICommand(CommandType::DECLARE), varName(varName), value(value) {}

    void execute(Process* process) override;

private:
    std::string varName;
    uint16_t    value;
};
