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

    // Memory management
    uint32_t     maxOverallMem     = 16384;
    uint32_t     memPerFrame       = 16;
    uint32_t     memPerProc        = 4096;
};

class ConfigParser
{
public:

    bool loadFromFile(const std::string& filepath);
    const SystemConfig& getConfig() const;

private:
    SystemConfig config;
    bool loaded = false;

};
