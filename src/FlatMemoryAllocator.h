#pragma once

#include "IMemoryAllocator.h"
#include <vector>
#include <map>
#include <mutex>
#include <cstdint>

class FlatMemoryAllocator : public IMemoryAllocator
{
public:
    FlatMemoryAllocator(uint32_t maxMem, uint32_t memPerFrame);

    void* allocate(size_t size) override;
    void  deallocate(void* ptr) override;
    std::string visualizeMemory() override;

    bool        allocateForProcess(const std::string& processName, size_t memSize) override;
    void        deallocateProcess(const std::string& processName) override;
    bool        hasAllocation(const std::string& processName) const override;

    bool readMemory(const std::string& processName, uint32_t address, uint16_t& outValue) override;
    bool writeMemory(const std::string& processName, uint32_t address, uint16_t value) override;

    int         getProcessCount() const override;
    uint32_t    getUsedMemory() const override;
    uint32_t    getTotalMemory() const override { return static_cast<uint32_t>(maximumSize); }
    uint32_t    getExternalFragmentation() const override;
    size_t      getProcessMemorySize(const std::string& processName) const override;

private:
    struct FlatBlock
    {
        uint32_t    startAddr;
        uint32_t    size;
        std::string processName; // empty = free
    };

    uint32_t memPerFrame;

    std::vector<FlatBlock> blocks;
    std::vector<uint8_t> physicalMemory; // simulated physical memory bytes
    std::map<std::string, size_t> processMemSizes; // track per-process memory size

    mutable std::mutex memMutex;

    void mergeFreeBlocks();

    // Find the block belonging to a process and check if address is in range
    bool isAddressValid(const std::string& processName, uint32_t address) const;
};
