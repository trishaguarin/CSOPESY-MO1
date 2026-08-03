#pragma once

#include <string>
#include <memory>

class Process;

class ICommand
{
public:
    enum CommandType
    {
        PRINT,
        DECLARE,
        ADD,
        SUBTRACT,
        SLEEP,
        FOR,
        MEM_READ,
        MEM_WRITE
    };

    ICommand(CommandType type) : type(type) {}
    virtual ~ICommand() = default;
    virtual void execute(Process* process) = 0;

    CommandType getType() const { return type; }

protected:
    CommandType type;
};
