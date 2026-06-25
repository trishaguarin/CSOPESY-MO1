// ============================================================================
// Instruction.h — Process Instruction Types
// ============================================================================
// MO1 REQUIREMENT: Barebones process instructions (page 2-3)
//   PRINT(msg)                        — display output to console
//   DECLARE(var, value)               — declare uint16 variable
//   ADD(var1, var2/value, var3/value)  — var1 = var2 + var3
//   SUBTRACT(var1, var2/value, var3/value) — var1 = var2 - var3
//   SLEEP(X)                          — sleep for X CPU ticks, relinquish CPU
//   FOR([instructions], repeats)      — loop, nestable up to 3 times
//
// NOTES from spec:
//   - Process instructions are pre-determined, not typed by user
//   - Randomized via scheduler-start command
//   - Variables stored in memory, released when process finishes
//   - uint16 clamped between [0, 65535]
//   - Default PRINT msg: "Hello world from <process_name>!"
//   - FOR loops can be nested up to 3 times
// ============================================================================
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

// A single instruction for a process to execute
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

// TODO: Instruction generator for scheduler-start
//   Generates a random sequence of instructions for a process
//   - Count between min-ins and max-ins (from config)
//   - Types are randomized
//   - FOR loops nested up to 3 times
// std::vector<Instruction> generateRandomInstructions(
//     int minIns, int maxIns, const std::string& processName);
