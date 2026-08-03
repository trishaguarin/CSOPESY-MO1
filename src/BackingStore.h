#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <atomic>

// Manages csopesy-backing-store.txt using an indexed record architecture.
// Thread-safe: all public methods are mutex-protected.
class BackingStore
{
public:
    BackingStore();

    // Page out: write frame data for a process page to the backing store file
    void pageOut(const std::string& processName, int pageIndex, const std::vector<uint8_t>& frameData);

    // Page in: read frame data for a process page from the backing store file
    // Returns empty vector if not found
    std::vector<uint8_t> pageIn(const std::string& processName, int pageIndex);

    // Remove all entries for a finished process
    void removeProcess(const std::string& processName);

    // Clear the entire backing store
    void clear();

    uint64_t getNumPagedIn() const { return numPagedIn.load(); }
    uint64_t getNumPagedOut() const { return numPagedOut.load(); }

private:
    static const std::string FILENAME;

    struct StoreEntry
    {
        std::string processName;
        int         pageIndex;
        std::vector<uint8_t> data;
    };

    // In-memory index of backing store entries
    std::vector<StoreEntry> entries;
    mutable std::mutex storeMutex;

    std::atomic<uint64_t> numPagedIn{0};
    std::atomic<uint64_t> numPagedOut{0};

    // Sync in-memory entries to the backing store file
    void flushToFile();
};
