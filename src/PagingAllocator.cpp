#include "PagingAllocator.h"
#include <sstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>

PagingAllocator::PagingAllocator(uint32_t maxMem, uint32_t memPerFrame)
    : memPerFrame(memPerFrame)
{
    memoryAllocatorType = PAGING;
    maximumSize = maxMem;
    currentAllocatedSize = 0;

    if (memPerFrame == 0 || memPerFrame > maxMem)
    {
        throw std::invalid_argument(
            "PagingAllocator: mem-per-frame must be > 0 and <= max-overall-mem");
    }

    numFrames = maxMem / memPerFrame;

    physicalMemory.assign(maxMem, 0);
    frameOwner.assign(numFrames, "");
}

void* PagingAllocator::allocate(size_t size) { return nullptr; }
void  PagingAllocator::deallocate(void* ptr) {}

std::string PagingAllocator::visualizeMemory()
{
    std::lock_guard<std::mutex> lock(memMutex);
    std::ostringstream ss;
    ss << "----end---- = " << maximumSize << "\n";

    // Walk frames from top to bottom, grouping contiguous frames by process
    int i = static_cast<int>(numFrames) - 1;
    while (i >= 0)
    {
        const std::string& owner = frameOwner[i];

        if (owner.empty())
        {
            ss << "\n";
            --i;
        }
        else
        {
            int top = i;
            while (i >= 0 && frameOwner[i] == owner)
                --i;
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

bool PagingAllocator::allocateForProcess(const std::string& processName, size_t memSize)
{
    std::lock_guard<std::mutex> lock(memMutex);

    // If already allocated, skip
    if (pageTables.count(processName) && !pageTables[processName].empty())
        return true;

    uint32_t pagesNeeded = static_cast<uint32_t>(memSize) / memPerFrame;
    if (pagesNeeded == 0) pagesNeeded = 1;

    // If physical RAM has only 1 frame total and is currently in use, new processes wait in ready queue
    if (numFrames == 1 && currentAllocatedSize > 0)
    {
        return false;
    }

    // Create page table for this process
    std::vector<PageTableEntry> pt(pagesNeeded);

    // Find free frames, evicting as needed (FIFO)
    for (uint32_t p = 0; p < pagesNeeded; ++p)
    {
        int frame = findFreeFrame();
        if (frame == -1)
        {
            evictPage();
            frame = findFreeFrame();
            if (frame == -1)
                return false; // truly out of memory
        }

        pt[p].frameNumber = frame;
        pt[p].isValid = true;
        pt[p].isDirty = false;

        frameOwner[frame] = processName;
        fifoQueue.push_back({processName, static_cast<int>(p)});
    }

    pageTables[processName] = pt;
    processMemSizes[processName] = memSize;
    currentAllocatedSize += pagesNeeded * memPerFrame;

    return true;
}

void PagingAllocator::deallocateProcess(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(memMutex);

    auto it = pageTables.find(processName);
    if (it == pageTables.end())
        return;

    // Free all frames
    for (auto& pte : it->second)
    {
        if (pte.isValid && pte.frameNumber >= 0)
        {
            std::memset(&physicalMemory[pte.frameNumber * memPerFrame], 0, memPerFrame);
            frameOwner[pte.frameNumber] = "";
            currentAllocatedSize -= memPerFrame;
        }
    }

    pageTables.erase(it);
    processMemSizes.erase(processName);

    // Remove from FIFO queue
    fifoQueue.erase(
        std::remove_if(fifoQueue.begin(), fifoQueue.end(),
            [&](const std::pair<std::string, int>& entry) {
                return entry.first == processName;
            }),
        fifoQueue.end()
    );

    // Remove from backing store
    backingStore.removeProcess(processName);
}

bool PagingAllocator::hasAllocation(const std::string& processName) const
{
    std::lock_guard<std::mutex> lock(memMutex);
    auto it = pageTables.find(processName);
    return it != pageTables.end() && !it->second.empty();
}

bool PagingAllocator::readMemory(const std::string& processName, uint32_t address, uint16_t& outValue)
{
    std::lock_guard<std::mutex> lock(memMutex);

    auto it = pageTables.find(processName);
    if (it == pageTables.end())
        return false;

    if (address + 1 >= 65536)
        return false; // access violation — out of 16-bit address range

    int pageIdx = getPageForAddress(address);
    if (pageIdx < 0)
        return false;

    if (pageIdx >= static_cast<int>(it->second.size()))
    {
        it->second.resize(pageIdx + 1);
    }

    // Handle page fault if needed (page not in RAM)
    ensurePageLoaded(processName, pageIdx);

    auto& pte = it->second[pageIdx];
    if (!pte.isValid || pte.frameNumber < 0)
        return false;

    int offset = getOffsetInPage(address);
    uint32_t physAddr = pte.frameNumber * memPerFrame + offset;

    outValue = static_cast<uint16_t>(physicalMemory[physAddr]) |
               (static_cast<uint16_t>(physicalMemory[physAddr + 1]) << 8);
    return true;
}

bool PagingAllocator::writeMemory(const std::string& processName, uint32_t address, uint16_t value)
{
    std::lock_guard<std::mutex> lock(memMutex);

    auto it = pageTables.find(processName);
    if (it == pageTables.end())
        return false;

    if (address + 1 >= 65536)
        return false;

    int pageIdx = getPageForAddress(address);
    if (pageIdx < 0)
        return false;

    if (pageIdx >= static_cast<int>(it->second.size()))
    {
        it->second.resize(pageIdx + 1);
    }

    ensurePageLoaded(processName, pageIdx);

    auto& pte = it->second[pageIdx];
    if (!pte.isValid || pte.frameNumber < 0)
        return false;

    int offset = getOffsetInPage(address);
    uint32_t physAddr = pte.frameNumber * memPerFrame + offset;

    physicalMemory[physAddr] = static_cast<uint8_t>(value & 0xFF);
    physicalMemory[physAddr + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    pte.isDirty = true;

    return true;
}

int PagingAllocator::getProcessCount() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    int count = 0;
    for (auto& [name, pt] : pageTables)
    {
        for (auto& pte : pt)
        {
            if (pte.isValid)
            {
                count++;
                break;
            }
        }
    }
    return count;
}

uint32_t PagingAllocator::getUsedMemory() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    uint32_t used = 0;
    for (uint32_t f = 0; f < numFrames; ++f)
        if (!frameOwner[f].empty())
            used += memPerFrame;
    return used;
}

uint32_t PagingAllocator::getExternalFragmentation() const
{
    // In paging, external fragmentation is 0
    return 0;
}

size_t PagingAllocator::getProcessMemorySize(const std::string& processName) const
{
    std::lock_guard<std::mutex> lock(memMutex);
    auto it = processMemSizes.find(processName);
    if (it != processMemSizes.end())
        return it->second;
    return 0;
}

// ── Internal Helpers ─────────────────────────────────────────────────────────

int PagingAllocator::findFreeFrame() const
{
    for (uint32_t f = 0; f < numFrames; ++f)
        if (frameOwner[f].empty())
            return static_cast<int>(f);
    return -1;
}

void PagingAllocator::evictPage()
{
    // FIFO page replacement: evict the oldest loaded page
    while (!fifoQueue.empty())
    {
        auto [procName, pageIdx] = fifoQueue.front();
        fifoQueue.pop_front();

        auto ptIt = pageTables.find(procName);
        if (ptIt == pageTables.end())
            continue;
        if (pageIdx >= static_cast<int>(ptIt->second.size()))
            continue;

        auto& pte = ptIt->second[pageIdx];
        if (!pte.isValid || pte.frameNumber < 0)
            continue; // already evicted, try next

        int frame = pte.frameNumber;

        // Save frame data to backing store
        std::vector<uint8_t> frameData(
            physicalMemory.begin() + frame * memPerFrame,
            physicalMemory.begin() + (frame + 1) * memPerFrame
        );
        backingStore.pageOut(procName, pageIdx, frameData);

        // Clear frame
        std::memset(&physicalMemory[frame * memPerFrame], 0, memPerFrame);
        frameOwner[frame] = "";
        currentAllocatedSize -= memPerFrame;

        // Mark page as invalid (in backing store)
        pte.isValid = false;
        pte.frameNumber = -1;
        pte.isDirty = false;

        return; // Successfully evicted one page
    }
}

void PagingAllocator::ensurePageLoaded(const std::string& processName, int pageIndex)
{
    auto ptIt = pageTables.find(processName);
    if (ptIt == pageTables.end())
        return;

    auto& pte = ptIt->second[pageIndex];
    if (pte.isValid)
        return; // already in RAM

    // Page fault — need to bring page in from backing store
    int frame = findFreeFrame();
    if (frame == -1)
    {
        evictPage();
        frame = findFreeFrame();
    }
    if (frame == -1)
        return; // should not happen

    // Try to load from backing store
    std::vector<uint8_t> data = backingStore.pageIn(processName, pageIndex);
    if (!data.empty())
    {
        // Copy data into frame
        std::memcpy(&physicalMemory[frame * memPerFrame], data.data(),
                     std::min(data.size(), static_cast<size_t>(memPerFrame)));
    }
    // else: new page, already zeroed

    frameOwner[frame] = processName;
    pte.frameNumber = frame;
    pte.isValid = true;
    pte.isDirty = false;
    currentAllocatedSize += memPerFrame;

    fifoQueue.push_back({processName, pageIndex});
}

int PagingAllocator::getPageForAddress(uint32_t address) const
{
    return static_cast<int>(address / memPerFrame);
}

int PagingAllocator::getOffsetInPage(uint32_t address) const
{
    return static_cast<int>(address % memPerFrame);
}
