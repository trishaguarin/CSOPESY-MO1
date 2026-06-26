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

static std::string stripQuotes(const std::string& value)
{
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
        return value.substr(1, value.size() - 2);
    return value;
}

static bool parseUint32(const std::string& token, uint32_t& outValue)
{
    try
    {
        size_t idx = 0;
        unsigned long long parsed = std::stoull(token, &idx);
        if (idx != token.size() || parsed > 0xFFFFFFFFull)
            return false;
        outValue = static_cast<uint32_t>(parsed);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool ConfigParser::loadFromFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open config file '" << filepath << "'.\n";
        loaded = false;
        return false;
    }

    SystemConfig parsedConfig;
    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line))
    {
        ++lineNumber;
        if (line.empty())
            continue;

        std::istringstream stream(line);
        std::string key;
        std::string value;
        if (!(stream >> key >> value))
        {
            std::cerr << "Error: Invalid config entry on line " << lineNumber << ".\n";
            loaded = false;
            return false;
        }

        if (key == "num-cpu")
        {
            uint32_t v;
            if (!parseUint32(value, v) || v < 1 || v > 128)
            {
                std::cerr << "Error: Invalid num-cpu value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.numCpu = static_cast<int>(v);
        }
        else if (key == "scheduler")
        {
            std::string algo = stripQuotes(value);
            if (algo != "fcfs" && algo != "rr")
            {
                std::cerr << "Error: Invalid scheduler value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.schedulerAlgo = algo;
        }
        else if (key == "quantum-cycles")
        {
            uint32_t v;
            if (!parseUint32(value, v) || v < 1)
            {
                std::cerr << "Error: Invalid quantum-cycles value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.quantumCycles = v;
        }
        else if (key == "batch-process-freq")
        {
            uint32_t v;
            if (!parseUint32(value, v) || v < 1)
            {
                std::cerr << "Error: Invalid batch-process-freq value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.batchProcessFreq = v;
        }
        else if (key == "min-ins")
        {
            uint32_t v;
            if (!parseUint32(value, v) || v < 1)
            {
                std::cerr << "Error: Invalid min-ins value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.minIns = v;
        }
        else if (key == "max-ins")
        {
            uint32_t v;
            if (!parseUint32(value, v) || v < 1)
            {
                std::cerr << "Error: Invalid max-ins value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.maxIns = v;
        }
        else if (key == "delays-per-exec")
        {
            uint32_t v;
            if (!parseUint32(value, v))
            {
                std::cerr << "Error: Invalid delays-per-exec value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.delaysPerExec = v;
        }
        else
        {
            std::cerr << "Error: Unknown config key '" << key << "' on line " << lineNumber << ".\n";
            loaded = false;
            return false;
        }
    }

    if (parsedConfig.minIns > parsedConfig.maxIns)
    {
        std::cerr << "Error: min-ins cannot be greater than max-ins.\n";
        loaded = false;
        return false;
    }

    config = parsedConfig;
    loaded = true;
    return true;
}

const SystemConfig& ConfigParser::getConfig() const
{
    return config;
}
