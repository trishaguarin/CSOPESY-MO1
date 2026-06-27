#pragma once

#include "ICommand.h"
#include <string>

class PrintCommand : public ICommand
{
public:
    PrintCommand(const std::string& message = "", const std::string& varName = "")
        : ICommand(CommandType::PRINT), message(message), varName(varName) {}

    void execute(Process* process) override;

private:
    std::string message;
    std::string varName;
};
