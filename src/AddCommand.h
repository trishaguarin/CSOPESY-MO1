#pragma once

#include "ICommand.h"
#include <string>

class AddCommand : public ICommand
{
public:
    AddCommand(const std::string& dest,
               const std::string& operand1,
               const std::string& operand2)
        : ICommand(CommandType::ADD),
          dest(dest), operand1(operand1), operand2(operand2) {}

    void execute(Process* process) override;

private:
    std::string dest;     
    std::string operand1;
    std::string operand2;
};
