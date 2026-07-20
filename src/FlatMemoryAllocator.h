#pragma once

#include "IMemoryAllocator.h"
#include <vector>
#include <mutex>
#include <cstdint>

class FlatMemoryAllocator : public IMemoryAllocator
{
public:
    FlatMemoryAllocator(uint32_t maxMem, uint32_t memPerProc, uint32_t memPerFrame);

    // IMemoryAllocator interface
    void* allocate(size_t size) override;
    void  deallocate(void* ptr) override;
    std::string visualizeMemory() override;

    // Convenience wrappers
    bool        allocateForProcess(const std::string& processName) override;
    void        deallocateProcess(const std::string& processName) override;
    bool        hasAllocation(const std::string& processName) const override;

    // Query
    int         getProcessCount() const override;
    uint32_t    getUsedMemory() const override;
    uint32_t    getTotalMemory() const override { return static_cast<uint32_t>(maximumSize); }
    uint32_t    getExternalFragmentation() const override;

private:
    struct FlatBlock
    {
        uint32_t    startAddr;
        uint32_t    size;
        std::string processName; // empty = free
    };

    uint32_t memPerProc;
    uint32_t memPerFrame;

    std::vector<FlatBlock> blocks;
    mutable std::mutex memMutex;

    void mergeFreeBlocks();
};
