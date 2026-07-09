#include "MemoryAllocator.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <set>

MemoryAllocator::MemoryAllocator(uint32_t maxMem, uint32_t memPerProc, uint32_t memPerFrame)
    : maxMem(maxMem), memPerProc(memPerProc), memPerFrame(memPerFrame)
{
    // Start with one big free block
    blocks.push_back({0, maxMem, ""});
}

bool MemoryAllocator::allocate(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(memMutex);

    // First-fit: scan blocks for the first free block >= memPerProc
    for (size_t i = 0; i < blocks.size(); ++i)
    {
        if (blocks[i].processName.empty() && blocks[i].size >= memPerProc)
        {
            // Found a fit
            if (blocks[i].size == memPerProc)
            {
                // Exact fit — just assign
                blocks[i].processName = processName;
            }
            else
            {
                // Split: [allocated | remaining free]
                MemBlock allocated;
                allocated.startAddr = blocks[i].startAddr;
                allocated.size = memPerProc;
                allocated.processName = processName;

                MemBlock remaining;
                remaining.startAddr = blocks[i].startAddr + memPerProc;
                remaining.size = blocks[i].size - memPerProc;
                remaining.processName = ""; // free

                blocks[i] = allocated;
                blocks.insert(blocks.begin() + i + 1, remaining);
            }
            return true;
        }
    }
    return false; // No fit found
}

bool MemoryAllocator::hasAllocation(const std::string& processName) const
{
    std::lock_guard<std::mutex> lock(memMutex);
    for (auto& b : blocks)
    {
        if (b.processName == processName)
            return true;
    }
    return false;
}

void MemoryAllocator::deallocate(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(memMutex);

    for (auto& block : blocks)
    {
        if (block.processName == processName)
        {
            block.processName = ""; // mark free
        }
    }
    mergeFreeBlocks();
}

void MemoryAllocator::mergeFreeBlocks()
{
    // Merge adjacent free blocks
    for (size_t i = 0; i + 1 < blocks.size(); )
    {
        if (blocks[i].processName.empty() && blocks[i + 1].processName.empty())
        {
            blocks[i].size += blocks[i + 1].size;
            blocks.erase(blocks.begin() + i + 1);
        }
        else
        {
            ++i;
        }
    }
}

int MemoryAllocator::getProcessCount() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    std::set<std::string> names;
    for (auto& b : blocks)
    {
        if (!b.processName.empty())
            names.insert(b.processName);
    }
    return static_cast<int>(names.size());
}

uint32_t MemoryAllocator::getUsedMemory() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    uint32_t used = 0;
    for (auto& b : blocks)
    {
        if (!b.processName.empty())
            used += b.size;
    }
    return used;
}

uint32_t MemoryAllocator::getExternalFragmentation() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    uint32_t fragmentation = 0;
    for (auto& b : blocks)
    {
        if (b.processName.empty())
        {
            fragmentation += b.size;
        }
    }
    return fragmentation;
}

std::string MemoryAllocator::getMemoryStamp() const
{
    std::lock_guard<std::mutex> lock(memMutex);

    std::ostringstream ss;

    ss << "----end---- = " << maxMem << "\n";

    // Print blocks from top (high address) to bottom (low address)
    for (int i = static_cast<int>(blocks.size()) - 1; i >= 0; --i)
    {
        const auto& b = blocks[i];
        uint32_t upper = b.startAddr + b.size;
        uint32_t lower = b.startAddr;

        if (b.processName.empty())
        {
            // Free block — just blank space
            ss << "\n";
        }
        else
        {
            // Occupied block
            ss << upper << "\n";
            ss << b.processName << "\n";
            ss << lower << "\n";
        }
    }

    ss << "----start---- = 0\n";

    return ss.str();
}
