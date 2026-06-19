#pragma once
#include <atomic>
#include <queue>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <algorithm>
#include "Process.h"

// First-Come-First-Serve (FCFS) Scheduler
// Multi-threaded design:
//   • 1 dedicated scheduler thread  – dispatches processes from the ready queue to free cores
//   • 1 worker thread per CPU core  – executes print commands for the assigned process

class Scheduler {
public:
    explicit Scheduler(int numCores)
        : numCores(numCores), running(false) {
        coreStatus.assign(numCores, false);  // false = idle
    }

    ~Scheduler() {
        stop();
    }

    // Add a process to the scheduler before or after start()
    void addProcess(std::shared_ptr<Process> p) {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(p);
        allProcesses.push_back(p);
        cv.notify_all();
    }

    void start() {
        running = true;

        // Spawn one worker thread per core
        for (int i = 0; i < numCores; ++i) {
            coreThreads.emplace_back(&Scheduler::coreWorker, this, i);
        }

        // Spawn the scheduler thread
        schedulerThread = std::thread(&Scheduler::schedulerWorker, this);
    }

    void stop() {
        running = false;
        cv.notify_all();
        if (schedulerThread.joinable()) schedulerThread.join();
        for (auto& t : coreThreads) {
            if (t.joinable()) t.join();
        }
        coreThreads.clear();
    }

    bool allFinished() const {
        std::lock_guard<std::mutex> lock(listMutex);
        return std::all_of(allProcesses.begin(), allProcesses.end(),
            [](const std::shared_ptr<Process>& p) {
                return p->getState() == Process::FINISHED;
            });
    }

    // ── Display helpers ──────────────────────────────────────────────────────

    // Collect running processes (snapshot)
    std::vector<std::shared_ptr<Process>> getRunningProcesses() const {
        std::lock_guard<std::mutex> lock(listMutex);
        std::vector<std::shared_ptr<Process>> result;
        for (auto& p : allProcesses) {
            if (p->getState() == Process::RUNNING) result.push_back(p);
        }
        return result;
    }

    // Collect finished processes (snapshot)
    std::vector<std::shared_ptr<Process>> getFinishedProcesses() const {
        std::lock_guard<std::mutex> lock(listMutex);
        std::vector<std::shared_ptr<Process>> result;
        for (auto& p : allProcesses) {
            if (p->getState() == Process::FINISHED) result.push_back(p);
        }
        return result;
    }

    int getNumCores() const { return numCores; }

private:
    // ── Scheduler thread: dispatches ready processes to idle cores ───────────
    void schedulerWorker() {
        while (running) {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return !running || (!readyQueue.empty() && hasIdleCore());
            });

            if (!running) break;

            while (!readyQueue.empty() && hasIdleCore()) {
                int core = getIdleCore();
                auto proc = readyQueue.front();
                readyQueue.pop();

                {
                    std::lock_guard<std::mutex> lk(coreMutex);
                    coreStatus[core] = true;       // mark core busy
                    coreProcess[core] = proc;
                }

                proc->setState(Process::RUNNING);
                proc->setAssignedCore(core);
                coreCV.notify_all();               // wake that core's worker
            }
        }
    }

    // ── Core worker thread: runs the process assigned to this core ───────────
    void coreWorker(int coreId) {
        while (running) {
            std::shared_ptr<Process> proc;

            // Wait until this core has a process assigned
            {
                std::unique_lock<std::mutex> lock(coreMutex);
                coreCV.wait(lock, [this, coreId] {
                    return !running || (coreStatus[coreId] && coreProcess.count(coreId));
                });
                if (!running) break;
                proc = coreProcess[coreId];
            }

            // Execute all remaining print commands for this process
            while (proc && !proc->isFinished()) {
                proc->executePrint(coreId);
                // Tiny yield to keep the system responsive; remove if not needed
                std::this_thread::yield();
            }

            // Core is now free — notify scheduler
            {
                std::lock_guard<std::mutex> lock(coreMutex);
                coreStatus[coreId] = false;
                coreProcess.erase(coreId);
            }
            cv.notify_all();   // let scheduler know a core freed up
        }
    }

    // ── Helpers ──────────────────────────────────────────────────────────────
    bool hasIdleCore() const {
        for (bool busy : coreStatus) if (!busy) return true;
        return false;
    }

    int getIdleCore() const {
        for (int i = 0; i < numCores; ++i) if (!coreStatus[i]) return i;
        return -1;
    }

    // ── Member data ──────────────────────────────────────────────────────────
    int numCores;
    std::atomic<bool> running;

    std::queue<std::shared_ptr<Process>>          readyQueue;
    std::vector<std::shared_ptr<Process>>         allProcesses;

    std::thread                                   schedulerThread;
    std::vector<std::thread>                      coreThreads;

    // Per-core state
    std::vector<bool>                             coreStatus;   // true = busy
    std::map<int, std::shared_ptr<Process>>       coreProcess;  // coreId → process

    mutable std::mutex  queueMutex;
    mutable std::mutex  listMutex;
    mutable std::mutex  coreMutex;
    std::condition_variable cv;       // scheduler wakeup
    std::condition_variable coreCV;   // core worker wakeup
};