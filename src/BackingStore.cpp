#include "BackingStore.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

const std::string BackingStore::FILENAME = "csopesy-backing-store.txt";

BackingStore::BackingStore()
{
    // Start with empty backing store
    clear();
}

void BackingStore::pageOut(const std::string& processName, int pageIndex, const std::vector<uint8_t>& frameData)
{
    std::lock_guard<std::mutex> lock(storeMutex);

    // Remove existing entry for this process/page if any
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
            [&](const StoreEntry& e) {
                return e.processName == processName && e.pageIndex == pageIndex;
            }),
        entries.end()
    );

    // Add new entry
    entries.push_back({processName, pageIndex, frameData});
    numPagedOut++;

    flushToFile();
}

std::vector<uint8_t> BackingStore::pageIn(const std::string& processName, int pageIndex)
{
    std::lock_guard<std::mutex> lock(storeMutex);

    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        if (it->processName == processName && it->pageIndex == pageIndex)
        {
            std::vector<uint8_t> data = it->data;
            entries.erase(it);
            numPagedIn++;
            flushToFile();
            return data;
        }
    }

    // Not found — return empty
    return {};
}

void BackingStore::removeProcess(const std::string& processName)
{
    std::lock_guard<std::mutex> lock(storeMutex);

    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
            [&](const StoreEntry& e) {
                return e.processName == processName;
            }),
        entries.end()
    );

    flushToFile();
}

void BackingStore::clear()
{
    std::lock_guard<std::mutex> lock(storeMutex);
    entries.clear();
    // Write empty file
    std::ofstream file(FILENAME, std::ios::trunc);
    file.close();
}

void BackingStore::flushToFile()
{
    // Write all entries to backing store file in indexed record format
    std::ofstream file(FILENAME, std::ios::trunc);
    if (!file.is_open())
        return;

    for (const auto& entry : entries)
    {
        file << "[PROCESS: " << entry.processName << "][PAGE: " << entry.pageIndex << "]\n";

        // Write data as hex bytes
        for (size_t i = 0; i < entry.data.size(); ++i)
        {
            file << std::hex << std::setw(2) << std::setfill('0')
                 << static_cast<int>(entry.data[i]);
            if ((i + 1) % 32 == 0)
                file << "\n";
            else
                file << " ";
        }
        if (!entry.data.empty() && entry.data.size() % 32 != 0)
            file << "\n";
        file << "\n";
    }

    file.close();
}
