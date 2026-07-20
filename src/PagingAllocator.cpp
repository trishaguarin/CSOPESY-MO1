#include "PagingAllocator.h"
#include <sstream>
#include <algorithm>

PagingAllocator::PagingAllocator(uint32_t maxMem, uint32_t memPerProc, uint32_t memPerFrame)
    : memPerProc(memPerProc), memPerFrame(memPerFrame)
{
    memoryAllocatorType = PAGING;
    maximumSize = maxMem;
    currentAllocatedSize = 0;

    numFrames = maxMem / memPerFrame;
    framesPerProc = memPerProc / memPerFrame;

    // Initialize all frames as free (empty string)
    frameTable.assign(numFrames, "");
}

// Raw pointer interface (spec requirement — not used by Scheduler directly)
void* PagingAllocator::allocate(size_t size)
{
    return nullptr;
}

void PagingAllocator::deallocate(void* ptr)
{
    // Not used — deallocateProcess is used instead
}

std::string PagingAllocator::visualizeMemory()
{
    std::lock_guard<std::mutex> lock(memMutex);

    std::ostringstream ss;
    ss << "----end---- = " << maximumSize << "\n";

    // Walk frames from top (high address) to bottom (low address)
    // Group contiguous frames belonging to the same process
    int i = static_cast<int>(numFrames) - 1;
    while (i >= 0)
    {
        const std::string& owner = frameTable[i];

        if (owner.empty())
        {
            // Free frame — skip (blank line)
            ss << "\n";
            --i;
        }
        else
        {
            // Find the contiguous run of frames belonging to this process
            int top = i;
            while (i >= 0 && frameTable[i] == owner)
            {
                --i;
            }
            int bottom = i + 1;

            uint32_t upperAddr = (static_cast<uint32_t>(top) + 1) * memPerFrame;
            uint32_t lowerAddr = static_cast<uint32_t>(bottom) * memPerFrame;

            ss << upperAddr << "\n";
            ss << owner << "\n";
            ss << lowerAddr << "\n";
        }
    }

    ss << "----start---- = 0\n";
    return ss.str();
}

bool PagingAllocator::allocateForProcess(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(memMutex);

    // Check if process is being paged back in from backing store
    if (backedOutProcesses.count(processName))
    {
        backedOutProcesses.erase(processName);
    }

    // If already has allocation, skip
    if (pageTable.count(processName) && !pageTable[processName].empty())
    {
        return true;
    }

    // Count free frames
    int freeFrames = countFreeFrames();

    // If not enough free frames, page out oldest processes until we have enough
    while (freeFrames < static_cast<int>(framesPerProc))
    {
        if (loadOrder.empty())
        {
            return false; // No one to page out — truly out of memory
        }
        pageOutOldest();
        freeFrames = countFreeFrames();
    }

    // Allocate frames (find free ones)
    std::vector<int> allocatedFrames;
    for (uint32_t f = 0; f < numFrames && allocatedFrames.size() < framesPerProc; ++f)
    {
        if (frameTable[f].empty())
        {
            frameTable[f] = processName;
            allocatedFrames.push_back(static_cast<int>(f));
        }
    }

    pageTable[processName] = allocatedFrames;
    loadOrder.push_back(processName);
    currentAllocatedSize += framesPerProc * memPerFrame;

    numPagedIn += framesPerProc;

    return true;
}

void PagingAllocator::deallocateProcess(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(memMutex);

    auto it = pageTable.find(processName);
    if (it == pageTable.end())
        return;

    // Free all frames
    for (int frameIdx : it->second)
    {
        frameTable[frameIdx] = "";
    }

    currentAllocatedSize -= static_cast<uint32_t>(it->second.size()) * memPerFrame;
    pageTable.erase(it);

    // Remove from load order
    loadOrder.erase(
        std::remove(loadOrder.begin(), loadOrder.end(), processName),
        loadOrder.end()
    );

    // Also remove from backed-out set if present
    backedOutProcesses.erase(processName);
}

bool PagingAllocator::hasAllocation(const std::string& processName) const
{
    std::lock_guard<std::mutex> lock(memMutex);
    auto it = pageTable.find(processName);
    return it != pageTable.end() && !it->second.empty();
}

int PagingAllocator::getProcessCount() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    int count = 0;
    for (auto& [name, frames] : pageTable)
    {
        if (!frames.empty())
            count++;
    }
    return count;
}

uint32_t PagingAllocator::getUsedMemory() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    uint32_t used = 0;
    for (uint32_t f = 0; f < numFrames; ++f)
    {
        if (!frameTable[f].empty())
            used += memPerFrame;
    }
    return used;
}

uint32_t PagingAllocator::getExternalFragmentation() const
{
    // In paging, there is no external fragmentation (any free frame can be used)
    return 0;
}

int PagingAllocator::countFreeFrames() const
{
    int count = 0;
    for (uint32_t f = 0; f < numFrames; ++f)
    {
        if (frameTable[f].empty())
            count++;
    }
    return count;
}

void PagingAllocator::pageOutOldest()
{
    // Find the oldest process in memory
    if (loadOrder.empty())
        return;

    std::string oldest = loadOrder.front();
    loadOrder.pop_front();

    auto it = pageTable.find(oldest);
    if (it == pageTable.end())
        return;

    // Free its frames
    uint32_t framesFreed = static_cast<uint32_t>(it->second.size());
    for (int frameIdx : it->second)
    {
        frameTable[frameIdx] = "";
    }
    currentAllocatedSize -= framesFreed * memPerFrame;

    numPagedOut += framesFreed;

    // Mark as backed out (process still exists, just not in physical memory)
    backedOutProcesses.insert(oldest);
    pageTable.erase(it);
}
