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
        else if (key == "delay-per-exec")
        {
            uint32_t v;
            if (!parseUint32(value, v))
            {
                std::cerr << "Error: Invalid delay-per-exec value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.delaysPerExec = v;
        }
        else if (key == "max-overall-mem")
        {
            uint32_t v;
            if (!parseUint32(value, v) || v < 1)
            {
                std::cerr << "Error: Invalid max-overall-mem value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.maxOverallMem = v;
        }
        else if (key == "mem-per-frame")
        {
            uint32_t v;
            if (!parseUint32(value, v) || v < 1)
            {
                std::cerr << "Error: Invalid mem-per-frame value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.memPerFrame = v;
        }
        else if (key == "mem-per-proc")
        {
            uint32_t v;
            if (!parseUint32(value, v) || v < 1)
            {
                std::cerr << "Error: Invalid mem-per-proc value on line " << lineNumber << ".\n";
                loaded = false;
                return false;
            }
            parsedConfig.memPerProc = v;
        }
        else
        {
            // Ignore unknown keys (forward compatibility)
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
