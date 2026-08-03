#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <algorithm>

class SymbolTable
{
public:
    static constexpr int MAX_VARIABLES = 32;   // 64 bytes / 2 bytes per uint16
    static constexpr int SEGMENT_SIZE  = 64;   // fixed symbol table segment size

    void setVariable(const std::string& name, int value)
    {
        // If variable already exists, update it
        auto it = table.find(name);
        if (it != table.end())
        {
            // Clamp to uint16 range [0, 65535]
            if (value < 0) value = 0;
            if (value > 65535) value = 65535;
            it->second = value;
            return;
        }

        // New variable — check cap
        if (static_cast<int>(table.size()) >= MAX_VARIABLES)
            return; // Ignore if at limit (per spec)

        // Clamp to uint16 range [0, 65535]
        if (value < 0) value = 0;
        if (value > 65535) value = 65535;
        table[name] = value;
    }

    int getVariable(const std::string& name)
    {
        // Auto-declare with 0 if not found (per MO1 spec)
        auto it = table.find(name);
        if (it == table.end())
        {
            if (static_cast<int>(table.size()) >= MAX_VARIABLES)
                return 0; // Can't add new variable — at limit
            table[name] = 0;
            return 0;
        }
        return it->second;
    }

    bool hasVariable(const std::string& name) const
    {
        return table.find(name) != table.end();
    }

    bool isFull() const
    {
        return static_cast<int>(table.size()) >= MAX_VARIABLES;
    }

    int getCount() const
    {
        return static_cast<int>(table.size());
    }

private:
    std::unordered_map<std::string, int> table;
};
