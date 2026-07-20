#include "Scheduler.h"
#include "FlatMemoryAllocator.h"
#include "PagingAllocator.h"
#include "PrintCommand.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <fstream>
#include <filesystem>

#include "DeclareCommand.h"
#include "AddCommand.h"
#include "SleepCommand.h"

Scheduler::Scheduler(const SystemConfig& config)
    : config(config)
{
    coreStatus.assign(config.numCpu, false);

    // Auto-detect allocator type:
    // If mem-per-frame == max-overall-mem, it's flat (one big frame = whole memory)
    // Otherwise, it's paging
    if (config.memPerFrame >= config.maxOverallMem)
    {
        memoryAllocator = std::make_unique<FlatMemoryAllocator>(
            config.maxOverallMem, config.memPerProc, config.memPerFrame);
    }
    else
    {
        memoryAllocator = std::make_unique<PagingAllocator>(
            config.maxOverallMem, config.memPerProc, config.memPerFrame);
    }
}

Scheduler::~Scheduler()
{
    stop();
}

// ── Lifecycle ────────────────────────────────────────────────────────────────

void Scheduler::start()
{
    if (running.load()) return;
    running = true;

    // Launch core worker threads
    for (int i = 0; i < config.numCpu; ++i)
    {
        coreTicksUsed[i] = 0;
        coreThreads.emplace_back(&Scheduler::coreWorker, this, i);
    }

    // Launch scheduler loop thread
    schedulerThread = std::thread(&Scheduler::schedulerLoop, this);
}

void Scheduler::stop()
{
    running = false;
    batchGenerating = false;
    schedulerCV.notify_all();
    coreCV.notify_all();

    if (schedulerThread.joinable()) schedulerThread.join();
    for (auto& t : coreThreads)
        if (t.joinable()) t.join();
    coreThreads.clear();
}

// ── Process Management ───────────────────────────────────────────────────────

void Scheduler::addProcess(std::shared_ptr<Process> proc)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(proc);
    }
    {
        std::lock_guard<std::mutex> lock(listMutex);
        allProcesses.push_back(proc);
    }
    schedulerCV.notify_all();
}

// ── Batch Generation ─────────────────────────────────────────────────────────

void Scheduler::startBatchGeneration()
{
    // Set lastBatchTime so the first process is generated immediately
    lastBatchTime = std::chrono::steady_clock::now() -
        std::chrono::seconds(config.batchProcessFreq);
    batchGenerating = true;
}

void Scheduler::stopBatchGeneration()
{
    batchGenerating = false;
}

// ── Query Methods ────────────────────────────────────────────────────────────

std::vector<std::shared_ptr<Process>> Scheduler::getRunningProcesses() const
{
    std::lock_guard<std::mutex> lock(listMutex);
    std::vector<std::shared_ptr<Process>> result;
    for (auto& p : allProcesses)
    {
        auto s = p->getState();
        if (s == Process::RUNNING || s == Process::WAITING)
            result.push_back(p);
    }
    return result;
}

std::vector<std::shared_ptr<Process>> Scheduler::getFinishedProcesses() const
{
    std::lock_guard<std::mutex> lock(listMutex);
    std::vector<std::shared_ptr<Process>> result;
    for (auto& p : allProcesses)
        if (p->getState() == Process::FINISHED)
            result.push_back(p);
    return result;
}

std::vector<std::shared_ptr<Process>> Scheduler::getAllProcesses() const
{
    std::lock_guard<std::mutex> lock(listMutex);
    return allProcesses;
}

std::shared_ptr<Process> Scheduler::findProcess(const std::string& name) const
{
    std::lock_guard<std::mutex> lock(listMutex);
    for (auto& p : allProcesses)
        if (p->getName() == name)
            return p;
    return nullptr;
}

int Scheduler::getNumCores() const { return config.numCpu; }

int Scheduler::getCoresUsed() const
{
    std::lock_guard<std::mutex> lock(coreMutex);
    return static_cast<int>(std::count(coreStatus.begin(), coreStatus.end(), true));
}

int Scheduler::getCoresAvailable() const
{
    return getNumCores() - getCoresUsed();
}

float Scheduler::getCpuUtilization() const
{
    std::lock_guard<std::mutex> lock(windowMutex);
    if (utilizationWindow.empty()) return 0.0f;

    double sum = 0.0;
    for (float v : utilizationWindow)
        sum += v;
    return static_cast<float>(sum / utilizationWindow.size());
}

uint64_t Scheduler::getCpuTicks() const
{
    return cpuTickCounter.load();
}

// ── Scheduler Loop ───────────────────────────────────────────────────────────

void Scheduler::schedulerLoop()
{
    lastBatchTime = std::chrono::steady_clock::now();

    while (running.load())
    {
        cpuTickCounter++;

        // ── Batch generation (time-based) ──
        if (batchGenerating.load())
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - lastBatchTime).count();
            if (elapsed >= static_cast<long long>(config.batchProcessFreq))
            {
                lastBatchTime = now;
                auto proc = generateProcess();
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    readyQueue.push(proc);
                }
                {
                    std::lock_guard<std::mutex> lock(listMutex);
                    allProcesses.push_back(proc);
                }
            }
        }

        // ── Handle sleeping processes ──
        {
            std::lock_guard<std::mutex> lock(listMutex);
            for (auto& p : allProcesses)
            {
                if (p->getState() == Process::WAITING)
                {
                    p->tickSleep();
                    // If it woke up, re-add to ready queue
                    if (p->getState() == Process::READY)
                    {
                        std::lock_guard<std::mutex> qlock(queueMutex);
                        readyQueue.push(p);
                    }
                }
            }
        }

        // ── Assign ready processes and Track utilization ──
        {
            std::lock_guard<std::mutex> lock(coreMutex);
            
            // 1. Assign ready processes to idle cores (with memory gating)
            for (int i = 0; i < config.numCpu; ++i)
            {
                if (!coreStatus[i])
                {
                    std::lock_guard<std::mutex> qlock(queueMutex);
                    if (readyQueue.empty()) break;

                    auto proc = readyQueue.front();
                    readyQueue.pop();

                    // Try to allocate memory for this process (skip if already has memory from preemption)
                    if (!memoryAllocator->hasAllocation(proc->getName()))
                    {
                        bool hasMemory = memoryAllocator->allocateForProcess(proc->getName());
                        if (!hasMemory)
                        {
                            // Memory full — push back to tail of ready queue
                            readyQueue.push(proc);
                            continue;
                        }
                    }

                    proc->setAssignedCore(i);          // 1. record which core
                    coreStatus[i] = true;              // 2. mark core busy
                    coreProcess[i] = proc;             // 3. register mapping
                    coreTicksUsed[i] = 0;
                    proc->setState(Process::RUNNING);  // 4. NOW make it visible as RUNNING
                }
            }
            
            // 2. Track utilization (sliding window) while we still hold the lock
            int busyCores = 0;
            for (int i = 0; i < config.numCpu; ++i)
            {
                if (coreStatus[i]) busyCores++;
            }
            float utilPct = (config.numCpu > 0)
                ? (static_cast<float>(busyCores) / config.numCpu) * 100.0f
                : 0.0f;

            std::lock_guard<std::mutex> wlock(windowMutex);
            utilizationWindow.push_back(utilPct);
            if ((int)utilizationWindow.size() > UTIL_WINDOW_SIZE)
                utilizationWindow.pop_front();
        }
        coreCV.notify_all();

        // ── Memory stamp every quantum-cycles ticks ──
        quantumCycleCount++;
        if (quantumCycleCount % config.quantumCycles == 0)
        {
            writeMemoryStamp();
        }

        // Small sleep to control tick rate and prevent host CPU hogging
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// ── Core Worker ──────────────────────────────────────────────────────────────

void Scheduler::coreWorker(int coreId)
{
    while (running.load())
    {
        std::shared_ptr<Process> proc;

        // Wait for process assignment
        {
            std::unique_lock<std::mutex> lock(coreMutex);
            coreCV.wait(lock, [&] {
                return !running.load() ||
                       (coreStatus[coreId] && coreProcess.count(coreId));
            });
            if (!running.load()) break;
            proc = coreProcess[coreId];
        }

        if (!proc) continue;

        // Execute commands
        uint32_t ticksUsed = 0;
        while (proc && !proc->isFinished() && running.load())
        {
            // Busy-wait delay (delays-per-exec)
            // Process stays on CPU but does no work
            bool preempted = false;
            for (uint32_t d = 0; d < config.delaysPerExec; ++d)
            {
                ticksUsed++;
                // RR: check quantum during delay too
                if (config.schedulerAlgo == "rr" && ticksUsed >= config.quantumCycles)
                {
                    preempted = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if (preempted)
                break;

            // Execute one command
            proc->executeCurrentCommand(coreId);
            ticksUsed++;


            if (proc->getState() == Process::WAITING)
            {
                // State is already WAITING — safe to clear core now
                proc->setAssignedCore(-1);
                break;
            }

            proc->moveToNextLine();

            // Check if finished after moveToNextLine
            if (proc->isFinished())
            {
                proc->setAssignedCore(-1);
                // Release memory when process finishes
                memoryAllocator->deallocateProcess(proc->getName());
                break;
            }

            // RR: check quantum after instruction execution
            if (config.schedulerAlgo == "rr" && ticksUsed >= config.quantumCycles)
            {
                break; 
            }
        }

        // Sleep once per quantum to simulate CPU time and keep core visibly busy
        // (Avoids per-instruction sleep which suffers from Windows ~15ms granularity)
        if (config.delaysPerExec == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(config.quantumCycles));
        }

    
        {
            
            if (proc && !proc->isFinished() && proc->getState() != Process::WAITING)
            {
                proc->setState(Process::READY);
                proc->setAssignedCore(-1);
                {
                    std::lock_guard<std::mutex> qlock(queueMutex);
                    readyQueue.push(proc); // ← re-queue BEFORE freeing core
                }
            }

            
            std::lock_guard<std::mutex> lock(coreMutex);
            coreStatus[coreId] = false;
            coreProcess.erase(coreId);
            coreTicksUsed[coreId] = 0;
        }
        schedulerCV.notify_all();
    }
}

// ── Process Generator ────────────────────────────────────────────────────────

std::shared_ptr<Process> Scheduler::generateProcess()
{
    processCounter++;

    // Name format: p01, p02, ..., p10, p100, etc.
    std::ostringstream nameStream;
    nameStream << "process" << std::setw(2) << std::setfill('0') << processCounter;
    std::string name = nameStream.str();

    // Random instruction count between min-ins and max-ins
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(config.minIns, config.maxIns);
    uint32_t numInstructions = dist(rng);

    auto proc = std::make_shared<Process>(processCounter, name);

    // Initialize variable "x" to 0
    proc->getSymbolTable().setVariable("x", 0);

    std::uniform_int_distribution<int> addDist(1, 10);

    for (uint32_t i = 0; i < numInstructions; ++i)
    {
        if (i % 2 == 0)
        {
            proc->addCommand(std::make_shared<PrintCommand>("Value from: ", "x"));
        }
        else
        {
            proc->addCommand(std::make_shared<AddCommand>("x", "x", std::to_string(addDist(rng))));
        }
    }

    return proc;
}

// ── Memory Stamp ─────────────────────────────────────────────────────────────

void Scheduler::writeMemoryStamp()
{
    uint64_t qq = quantumCycleCount / config.quantumCycles;

    std::filesystem::create_directories("MEM_STAMPS");

    std::ostringstream filename;
    filename << "MEM_STAMPS/memory_stamp_" << qq << ".txt";

    // Get timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif


    std::ostringstream ts;
    ts << "(" << std::setw(2) << std::setfill('0') << (tm_now.tm_mon + 1) << "/"
       << std::setw(2) << std::setfill('0') << tm_now.tm_mday << "/"
       << (tm_now.tm_year + 1900) << " "
       << std::setw(2) << std::setfill('0') << ((tm_now.tm_hour % 12 == 0) ? 12 : tm_now.tm_hour % 12) << ":"
       << std::setw(2) << std::setfill('0') << tm_now.tm_min << ":"
       << std::setw(2) << std::setfill('0') << tm_now.tm_sec
       << (tm_now.tm_hour >= 12 ? "PM" : "AM") << ")";

    int procCount = memoryAllocator->getProcessCount();
    uint32_t extFrag = memoryAllocator->getExternalFragmentation();

    std::ofstream file(filename.str());
    if (file.is_open())
    {
        file << "Timestamp: " << ts.str() << "\n";
        file << "Number of processes in memory: " << procCount << "\n\n";
        file << "Total external fragmentation in KB: " << (extFrag / 1024) << "\n";
        file << "num-pages-in: " << memoryAllocator->getNumPagedIn() << "\n";
        file << "num-pages-out: " << memoryAllocator->getNumPagedOut() << "\n\n";
        file << memoryAllocator->visualizeMemory();
        file.close();
    }
}
