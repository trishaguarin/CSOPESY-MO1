#pragma once

#include "IMemoryAllocator.h"
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <string>
#include <deque>

class PagingAllocator : public IMemoryAllocator
{
public:
    PagingAllocator(uint32_t maxMem, uint32_t memPerProc, uint32_t memPerFrame);

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

    // Paging stats
    uint64_t    getNumPagedIn() const override { return numPagedIn.load(); }
    uint64_t    getNumPagedOut() const override { return numPagedOut.load(); }

private:
    uint32_t memPerProc;
    uint32_t memPerFrame;
    uint32_t numFrames;
    uint32_t framesPerProc;

    // Frame table: frameIndex -> processName (empty = free)
    std::vector<std::string> frameTable;

    // Per-process page table: processName -> list of frame indices
    std::map<std::string, std::vector<int>> pageTable;

    // Track the order processes were loaded (for oldest-first page-out)
    std::deque<std::string> loadOrder;

    // Backing store: processes that have been paged out
    std::set<std::string> backedOutProcesses;

    // Page-in / page-out counters
    std::atomic<uint64_t> numPagedIn{0};
    std::atomic<uint64_t> numPagedOut{0};

    mutable std::mutex memMutex;

    // Internal helpers
    int countFreeFrames() const;
    void pageOutOldest();
};
