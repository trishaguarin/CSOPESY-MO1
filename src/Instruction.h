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

    // TODO: Fields for each instruction type. Possible approaches:
    //
    // Option A: Tagged union / variant
    //   Use std::variant or a union with the type enum as discriminant
    //
    // Option B: Flat struct with optional fields (simpler)
    //   std::string msg;                  // PRINT: the message to print
    //   std::string varName;              // DECLARE/ADD/SUBTRACT: target variable
    //   std::string operand1;             // ADD/SUBTRACT: second operand (var name or literal)
    //   std::string operand2;             // ADD/SUBTRACT: third operand (var name or literal)
    //   uint16_t    value = 0;            // DECLARE: default value
    //   uint8_t     sleepTicks = 0;       // SLEEP: number of CPU ticks
    //   std::vector<Instruction> body;    // FOR: nested instructions
    //   int         repeatCount = 0;      // FOR: number of repetitions

    // TODO: Static factory methods or a builder for creating instructions
    //   static Instruction makePrint(const std::string& processName);
    //   static Instruction makeDeclare(const std::string& var, uint16_t val);
    //   static Instruction makeAdd(const std::string& dest, const std::string& op1, const std::string& op2);
    //   static Instruction makeSubtract(const std::string& dest, const std::string& op1, const std::string& op2);
    //   static Instruction makeSleep(uint8_t ticks);
    //   static Instruction makeFor(const std::vector<Instruction>& body, int repeats);
};

// TODO: Instruction generator for scheduler-start
//   Generates a random sequence of instructions for a process
//   - Count between min-ins and max-ins (from config)
//   - Types are randomized
//   - FOR loops nested up to 3 times
// std::vector<Instruction> generateRandomInstructions(
//     int minIns, int maxIns, const std::string& processName);
