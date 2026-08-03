#include "FlatMemoryAllocator.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <set>
#include <cstring>

FlatMemoryAllocator::FlatMemoryAllocator(uint32_t maxMem, uint32_t memPerFrame)
    : memPerFrame(memPerFrame)
{
    memoryAllocatorType = FLAT_MEMORY_ALLOCATOR;
    maximumSize = maxMem;
    currentAllocatedSize = 0;

    blocks.push_back({0, maxMem, ""});
    physicalMemory.assign(maxMem, 0); // zero-initialized simulated memory
}

void* FlatMemoryAllocator::allocate(size_t size) { return nullptr; }
void  FlatMemoryAllocator::deallocate(void* ptr) {}

std::string FlatMemoryAllocator::visualizeMemory()
{
    std::lock_guard<std::mutex> lock(memMutex);
    std::ostringstream ss;
    ss << "----end---- = " << maximumSize << "\n";

    for (int i = static_cast<int>(blocks.size()) - 1; i >= 0; --i)
    {
        const auto& b = blocks[i];
        uint32_t upper = b.startAddr + b.size;
        uint32_t lower = b.startAddr;

        if (b.processName.empty())
        {
            ss << "\n";
        }
        else
        {
            ss << upper << "\n";
            ss << b.processName << "\n";
            ss << lower << "\n";
        }
    }

    ss << "----start---- = 0\n";
    return ss.str();
}

bool FlatMemoryAllocator::allocateForProcess(const std::string& processName, size_t memSize)
{
    std::lock_guard<std::mutex> lock(memMutex);

    uint32_t allocSize = static_cast<uint32_t>(memSize);

    for (size_t i = 0; i < blocks.size(); ++i)
    {
        if (blocks[i].processName.empty() && blocks[i].size >= allocSize)
        {
            if (blocks[i].size == allocSize)
            {
                blocks[i].processName = processName;
            }
            else
            {
                FlatBlock allocated;
                allocated.startAddr = blocks[i].startAddr;
                allocated.size = allocSize;
                allocated.processName = processName;

                FlatBlock remaining;
                remaining.startAddr = blocks[i].startAddr + allocSize;
                remaining.size = blocks[i].size - allocSize;
                remaining.processName = "";

                blocks[i] = allocated;
                blocks.insert(blocks.begin() + i + 1, remaining);
            }
            currentAllocatedSize += allocSize;
            processMemSizes[processName] = memSize;
            return true;
        }
    }
    return false;
}

void FlatMemoryAllocator::deallocateProcess(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(memMutex);

    for (auto& block : blocks)
    {
        if (block.processName == processName)
        {
            currentAllocatedSize -= block.size;
            // Zero out the freed memory
            std::memset(&physicalMemory[block.startAddr], 0, block.size);
            block.processName = "";
        }
    }
    processMemSizes.erase(processName);
    mergeFreeBlocks();
}

bool FlatMemoryAllocator::hasAllocation(const std::string& processName) const
{
    std::lock_guard<std::mutex> lock(memMutex);
    for (auto& b : blocks)
    {
        if (b.processName == processName)
            return true;
    }
    return false;
}

bool FlatMemoryAllocator::isAddressValid(const std::string& processName, uint32_t address) const
{
    for (auto& b : blocks)
    {
        if (b.processName == processName)
        {
            if (address >= b.startAddr && address + 1 < b.startAddr + b.size)
                return true;
        }
    }
    return false;
}

bool FlatMemoryAllocator::readMemory(const std::string& processName, uint32_t address, uint16_t& outValue)
{
    std::lock_guard<std::mutex> lock(memMutex);

    if (!isAddressValid(processName, address))
        return false; // access violation

    // Read uint16 (2 bytes, little-endian)
    outValue = static_cast<uint16_t>(physicalMemory[address]) |
               (static_cast<uint16_t>(physicalMemory[address + 1]) << 8);
    return true;
}

bool FlatMemoryAllocator::writeMemory(const std::string& processName, uint32_t address, uint16_t value)
{
    std::lock_guard<std::mutex> lock(memMutex);

    if (!isAddressValid(processName, address))
        return false; // access violation

    physicalMemory[address] = static_cast<uint8_t>(value & 0xFF);
    physicalMemory[address + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    return true;
}

void FlatMemoryAllocator::mergeFreeBlocks()
{
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

int FlatMemoryAllocator::getProcessCount() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    std::set<std::string> names;
    for (auto& b : blocks)
        if (!b.processName.empty())
            names.insert(b.processName);
    return static_cast<int>(names.size());
}

uint32_t FlatMemoryAllocator::getUsedMemory() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    return static_cast<uint32_t>(currentAllocatedSize);
}

uint32_t FlatMemoryAllocator::getExternalFragmentation() const
{
    std::lock_guard<std::mutex> lock(memMutex);
    uint32_t frag = 0;
    for (auto& b : blocks)
        if (b.processName.empty())
            frag += b.size;
    return frag;
}

size_t FlatMemoryAllocator::getProcessMemorySize(const std::string& processName) const
{
    std::lock_guard<std::mutex> lock(memMutex);
    auto it = processMemSizes.find(processName);
    if (it != processMemSizes.end())
        return it->second;
    return 0;
}
