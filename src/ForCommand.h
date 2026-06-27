#pragma once

#include "ICommand.h"
#include <vector>
#include <memory>

class ForCommand : public ICommand
{
public:
    ForCommand(const std::vector<std::shared_ptr<ICommand>>& body, int repeatCount)
        : ICommand(CommandType::FOR), body(body), repeatCount(repeatCount) {}

    void execute(Process* process) override;

private:
    std::vector<std::shared_ptr<ICommand>> body;
    int repeatCount;
};
