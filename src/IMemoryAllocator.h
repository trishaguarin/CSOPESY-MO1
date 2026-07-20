#pragma once

#include <string>
#include <cstdint>

// Abstract base class matching the spec interface from the Week 9-11 lecture.
class IMemoryAllocator
{
public:
    enum MemoryAllocatorType
    {
        FLAT_MEMORY_ALLOCATOR,
        PAGING,
    };

    virtual ~IMemoryAllocator() = default;

    // Core interface (from spec)
    virtual void* allocate(size_t size) = 0;
    virtual void  deallocate(void* ptr) = 0;
    virtual std::string visualizeMemory() = 0;

    // Convenience wrappers used by the Scheduler
    virtual bool        allocateForProcess(const std::string& processName) = 0;
    virtual void        deallocateProcess(const std::string& processName) = 0;
    virtual bool        hasAllocation(const std::string& processName) const = 0;

    // Query methods
    virtual int         getProcessCount() const = 0;
    virtual uint32_t    getUsedMemory() const = 0;
    virtual uint32_t    getTotalMemory() const = 0;
    virtual uint32_t    getExternalFragmentation() const = 0;

    // Paging stats (flat allocator returns 0)
    virtual uint64_t    getNumPagedIn() const { return 0; }
    virtual uint64_t    getNumPagedOut() const { return 0; }

    MemoryAllocatorType getType() const { return memoryAllocatorType; }

protected:
    MemoryAllocatorType memoryAllocatorType;

    struct MemoryBlock
    {
        size_t start;
        size_t size;

        bool operator<(const MemoryBlock& other) const
        {
            return start < other.start;
        }
    };

    size_t maximumSize = 0;
    size_t currentAllocatedSize = 0;
};
