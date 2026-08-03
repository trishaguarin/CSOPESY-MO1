#pragma once

#include "Process.h"
#include "ConfigParser.h"
#include "IMemoryAllocator.h"

#include <queue>
#include <vector>
#include <map>
#include <deque>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

class Scheduler
{
public:
    explicit Scheduler(const SystemConfig& config);
    ~Scheduler();

    // ── Lifecycle ────────────────────────────────────────────────────────
    void start();  // launch scheduler thread + core worker threads
    void stop();   // signal all threads to stop and join

    // ── Process Management ───────────────────────────────────────────────
    void addProcess(std::shared_ptr<Process> proc);

    // ── Batch Generation ─────────────────────────────────────────────────
    void startBatchGeneration();
    void stopBatchGeneration();

    // ── Query ────────────────────────────────────────────────────────────
    std::vector<std::shared_ptr<Process>> getRunningProcesses()  const;
    std::vector<std::shared_ptr<Process>> getFinishedProcesses() const;
    std::vector<std::shared_ptr<Process>> getAllProcesses()      const;
    std::shared_ptr<Process> findProcess(const std::string& name) const;

    int  getNumCores()       const;
    int  getCoresUsed()      const;
    int  getCoresAvailable() const;

    float getCpuUtilization() const;

    uint64_t getCpuTicks() const;

    const SystemConfig& getConfig() const { return config; }
    IMemoryAllocator* getMemoryAllocator() const { return memoryAllocator.get(); }

    // ── CPU Tick Counters for vmstat ──────────────────────────────────────
    uint64_t getIdleCpuTicks()   const { return idleCpuTicks.load(); }
    uint64_t getActiveCpuTicks() const { return activeCpuTicks.load(); }
    uint64_t getTotalCpuTicks()  const { return idleCpuTicks.load() + activeCpuTicks.load(); }

private:
    void schedulerLoop();
    void coreWorker(int coreId);


    std::shared_ptr<Process> generateProcess();

    // ── Configuration ────────────────────────────────────────────────────
    SystemConfig config;

    // ── Memory Allocator ─────────────────────────────────────────────────
    std::unique_ptr<IMemoryAllocator> memoryAllocator;

    // ── State ────────────────────────────────────────────────────────────
    std::atomic<bool>     running{false};
    std::atomic<bool>     batchGenerating{false};
    std::atomic<uint64_t> cpuTickCounter{0};

    // ── CPU tick counters for vmstat ──────────────────────────────────────
    std::atomic<uint64_t> idleCpuTicks{0};
    std::atomic<uint64_t> activeCpuTicks{0};

    // ── Process Queues ───────────────────────────────────────────────────
    std::queue<std::shared_ptr<Process>>   readyQueue;
    std::vector<std::shared_ptr<Process>>  allProcesses;

    // ── Threading ────────────────────────────────────────────────────────
    std::thread              schedulerThread;
    std::vector<std::thread> coreThreads;

    // ── Per-Core State ───────────────────────────────────────────────────
    std::vector<bool>                              coreStatus;   // true = busy
    std::map<int, std::shared_ptr<Process>>        coreProcess;  // coreId → process
    std::map<int, uint32_t> coreTicksUsed;

    // ── Synchronization ──────────────────────────────────────────────────
    mutable std::mutex       queueMutex;
    mutable std::mutex       listMutex;
    mutable std::mutex       coreMutex;
    std::condition_variable  schedulerCV;
    std::condition_variable  coreCV;

    // ── Batch generation ─────────────────────────────────────────────────
    int processCounter = 0;
    std::chrono::steady_clock::time_point lastBatchTime;

    // ── Memory stamp tracking ────────────────────────────────────────────
    uint64_t quantumCycleCount = 0;
    void writeMemoryStamp();

    // ── CPU utilization tracking ─────────────────────────────────────────
    static constexpr int UTIL_WINDOW_SIZE = 50;
    mutable std::mutex   windowMutex;
    std::deque<float>    utilizationWindow;
};
