#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class InstructionType
{
    PRINT,
    DECLARE,
    ADD,
    SUBTRACT,
    SLEEP,
    FOR
};

struct Instruction
{
    InstructionType type;

    std::string msg;
    std::string varName;
    std::string operand1;
    std::string operand2;
    uint16_t    value = 0;
    uint32_t    sleepTicks = 0;
    std::vector<Instruction> body;
    int         repeatCount = 0;

    static Instruction makePrint(const std::string& message = "")
    {
        Instruction i;
        i.type = InstructionType::PRINT;
        i.msg = message;
        return i;
    }

    static Instruction makeDeclare(const std::string& var, uint16_t val)
    {
        Instruction i;
        i.type = InstructionType::DECLARE;
        i.varName = var;
        i.value = val;
        return i;
    }

    static Instruction makeAdd(const std::string& dest,
                               const std::string& op1,
                               const std::string& op2)
    {
        Instruction i;
        i.type = InstructionType::ADD;
        i.varName = dest;
        i.operand1 = op1;
        i.operand2 = op2;
        return i;
    }

    static Instruction makeSubtract(const std::string& dest,
                                    const std::string& op1,
                                    const std::string& op2)
    {
        Instruction i;
        i.type = InstructionType::SUBTRACT;
        i.varName = dest;
        i.operand1 = op1;
        i.operand2 = op2;
        return i;
    }

    static Instruction makeSleep(uint32_t ticks)
    {
        Instruction i;
        i.type = InstructionType::SLEEP;
        i.sleepTicks = ticks;
        return i;
    }

    static Instruction makeFor(const std::vector<Instruction>& bodyInstructions,
                               int repeats)
    {
        Instruction i;
        i.type = InstructionType::FOR;
        i.body = bodyInstructions;
        i.repeatCount = repeats;
        return i;
    }
};