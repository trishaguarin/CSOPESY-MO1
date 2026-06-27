#pragma once

#include "ICommand.h"
#include <string>

class PrintCommand : public ICommand
{
public:
    PrintCommand(const std::string& message = "")
        : ICommand(CommandType::PRINT), message(message) {}

    void execute(Process* process) override;

private:
    std::string message;
};
