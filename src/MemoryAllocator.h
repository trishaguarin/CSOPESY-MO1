#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <cstdint>

class MemoryAllocator
{
public:
    MemoryAllocator(uint32_t maxMem, uint32_t memPerProc, uint32_t memPerFrame);

    // Returns true if allocation succeeded (first-fit)
    bool allocate(const std::string& processName);

    // Check if a process already has memory allocated
    bool hasAllocation(const std::string& processName) const;

    // Free memory held by a process
    void deallocate(const std::string& processName);

    // Number of unique processes currently in memory
    int getProcessCount() const;

    // Total bytes used by processes
    uint32_t getUsedMemory() const;

    // Total memory
    uint32_t getTotalMemory() const { return maxMem; }

    // External fragmentation: total free memory that cannot be used
    // (sum of all free blocks, since any free block < memPerProc is unusable)
    uint32_t getExternalFragmentation() const;

    // Generate the ASCII memory stamp string
    std::string getMemoryStamp() const;

private:
    struct MemBlock
    {
        uint32_t    startAddr;
        uint32_t    size;
        std::string processName; // empty = free
    };

    uint32_t maxMem;
    uint32_t memPerProc;
    uint32_t memPerFrame;

    std::vector<MemBlock> blocks;
    mutable std::mutex memMutex;

    void mergeFreeBlocks();
};
