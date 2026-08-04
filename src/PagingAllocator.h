#pragma once

#include "IMemoryAllocator.h"
#include "BackingStore.h"
#include <vector>
#include <map>
#include <deque>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <string>

class PagingAllocator : public IMemoryAllocator
{
public:
    PagingAllocator(uint32_t maxMem, uint32_t memPerFrame);

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
    size_t      getResidentMemory(const std::string& processName) const override;

    uint64_t    getNumPagedIn() const override { return backingStore.getNumPagedIn(); }
    uint64_t    getNumPagedOut() const override { return backingStore.getNumPagedOut(); }

private:
    // Page Table Entry (from lecture)
    struct PageTableEntry
    {
        int  frameNumber = -1;   // physical frame if valid
        bool isValid     = false; // true = in RAM
        bool isDirty     = false; // true = modified since loaded
    };

    uint32_t memPerFrame;
    uint32_t numFrames;

    // Simulated physical memory
    std::vector<uint8_t> physicalMemory;

    // Frame table: frameIndex -> owning process name (empty = free)
    std::vector<std::string> frameOwner;

    // Per-process page tables
    std::map<std::string, std::vector<PageTableEntry>> pageTables;

    // Per-process memory sizes
    std::map<std::string, size_t> processMemSizes;

    // FIFO queue for page replacement — tracks (processName, pageIndex) in load order
    std::deque<std::pair<std::string, int>> fifoQueue;

    // Backing store for paged-out data
    mutable BackingStore backingStore;

    mutable std::mutex memMutex;

    // Internal helpers
    int findFreeFrame() const;
    void evictPage(); // FIFO page replacement
    void ensurePageLoaded(const std::string& processName, int pageIndex);
    int getPageForAddress(uint32_t address) const;
    int getOffsetInPage(uint32_t address) const;
};
