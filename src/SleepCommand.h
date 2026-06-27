#pragma once

#include "ICommand.h"
#include <cstdint>

class SleepCommand : public ICommand
{
public:
    SleepCommand(uint8_t ticks)
        : ICommand(CommandType::SLEEP), ticks(ticks) {}

    void execute(Process* process) override;

private:
    uint8_t ticks; // number of CPU ticks to sleep
};
