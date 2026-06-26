#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <algorithm>

class SymbolTable
{
public:
    void setVariable(const std::string& name, int value)
    {
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
            table[name] = 0;
            return 0;
        }
        return it->second;
    }

    bool hasVariable(const std::string& name) const
    {
        return table.find(name) != table.end();
    }

private:
    std::unordered_map<std::string, int> table;
};
