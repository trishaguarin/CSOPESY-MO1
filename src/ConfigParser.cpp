// ============================================================================
// ConfigParser.cpp — config.txt Reader Implementation
// ============================================================================
// MO1 REQUIREMENT: Configuration setting
//   Parse the space-separated config.txt file.
//
// REFERENCE: The old Config.h in FCFS-scheduler/ had hardcoded values:
//   CORE_COUNT = 4, PROCESS_COUNT = 10, PRINTS_PER_PROCESS = 100
//   These must now come from config.txt at runtime.
// ============================================================================

#include "ConfigParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool ConfigParser::loadFromFile(const std::string& filepath)
{
    // TODO: Implement config.txt parsing
    //
    // Pseudocode:
    //   1. Open file at 'filepath'
    //   2. For each line:
    //      a. Split into key and value (space-separated)
    //      b. Match key to SystemConfig field:
    //         "num-cpu"            → config.numCpu
    //         "scheduler"          → config.schedulerAlgo (strip quotes)
    //         "quantum-cycles"     → config.quantumCycles
    //         "batch-process-freq" → config.batchProcessFreq
    //         "min-ins"            → config.minIns
    //         "max-ins"            → config.maxIns
    //         "delays-per-exec"    → config.delaysPerExec
    //      c. Validate ranges:
    //         num-cpu: [1, 128]
    //         quantum-cycles: [1, 2^32]
    //         etc.
    //   3. Set loaded = true on success
    //   4. Return false if file not found or parse error

    loaded = false; // TODO: set to true after successful parse
    return loaded;
}

const SystemConfig& ConfigParser::getConfig() const
{
    return config;
}
