// ============================================================================
// ConfigParser.h — config.txt Reader
// ============================================================================
// MO1 REQUIREMENT: Configuration setting
//   "The 'initialize' command should read from a 'config.txt' file,
//    the parameters for your CPU scheduler and process attributes."
//
// MO1 REQUIREMENT: config.txt format (page 5)
//   num-cpu             [1, 128]
//   scheduler           "fcfs" or "rr"
//   quantum-cycles      [1, 2^32]
//   batch-process-freq  [1, 2^32]
//   min-ins             [1, 2^32]
//   max-ins             [1, 2^32]
//   delays-per-exec     [0, 2^32]
// ============================================================================
#pragma once

#include <string>
#include <cstdint>

// Holds all runtime configuration parsed from config.txt
struct SystemConfig
{
    int          numCpu            = 4;
    std::string  schedulerAlgo     = "fcfs";  // "fcfs" or "rr"
    uint32_t     quantumCycles     = 5;
    uint32_t     batchProcessFreq  = 1;
    uint32_t     minIns            = 1000;
    uint32_t     maxIns            = 2000;
    uint32_t     delaysPerExec     = 0;
};

class ConfigParser
{
public:
    // TODO: Parse config.txt and populate a SystemConfig struct
    //   - Open "config.txt"
    //   - Read line-by-line, split by space
    //   - Map key → field in SystemConfig
    //   - Strip quotes from scheduler value ("fcfs" → fcfs)
    //   - Validate ranges per spec
    //   - Return false if file not found or invalid
    bool loadFromFile(const std::string& filepath);

    // Returns the parsed config (only valid after successful loadFromFile)
    const SystemConfig& getConfig() const;

private:
    SystemConfig config;
    bool loaded = false;

    // TODO: Helper to parse a single key-value line
    // void parseLine(const std::string& key, const std::string& value);
};
