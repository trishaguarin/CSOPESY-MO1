// ============================================================================
// Instruction.h — DEPRECATED: See ICommand.h
// ============================================================================
// This file has been superseded by the ICommand interface pattern
// taught in the CSOPESY midterm review lessons.
//
// LESSON REFERENCE: Midterm Review — "Representing process instructions"
//   "The general idea is to create a command/instruction interface that
//    all other instructions will implement."
//
// The new design uses:
//   - ICommand.h         — abstract base class with virtual execute()
//   - PrintCommand.h     — PRINT(msg)
//   - DeclareCommand.h   — DECLARE(var, value)
//   - AddCommand.h       — ADD(var1, var2, var3)
//   - SubtractCommand.h  — SUBTRACT(var1, var2, var3)
//   - SleepCommand.h     — SLEEP(X)
//   - ForCommand.h       — FOR([instructions], repeats)
//
// This file is kept for reference only. DO NOT USE in new code.
// ============================================================================
#pragma once

// Redirect to new interface
#include "ICommand.h"
