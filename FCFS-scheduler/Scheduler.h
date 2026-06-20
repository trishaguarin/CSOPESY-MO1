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
// a scheduler thread assigns processes to the worker threads
// a worker thread is assigned per CPU core, prints commands
// whenever a core is free, the worker thread notifies the scheduler to assign the next process
// repeats until queue is empty

class Scheduler {
public:
    explicit Scheduler(int numCores)
        : numCores(numCores), running(false) {
        coreStatus.assign(numCores, false);  // false = idle
    }

    ~Scheduler() {
        stop();
    }

    // add process to queue
    void addProcess(std::shared_ptr<Process> p)
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            readyQueue.push(p);
        }

        {
            std::lock_guard<std::mutex> lock(listMutex);
            allProcesses.push_back(p);
        }

        cv.notify_all();
    }

    void start() {
        running = true;

        // make worker thread per core
        for (int i = 0; i < numCores; ++i) {
            coreThreads.emplace_back(&Scheduler::coreWorker, this, i);
        }

        // makes scheduler thread
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

    // HELPER FOR UI ---
    // running
    std::vector<std::shared_ptr<Process>> getRunningProcesses() const {
        std::lock_guard<std::mutex> lock(listMutex);
        std::vector<std::shared_ptr<Process>> result;
        for (auto& p : allProcesses) {
            if (p->getState() == Process::RUNNING) result.push_back(p);
        }
        return result;
    }

    // finished
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
    // scheduler thread, assigns processes to cores
    void schedulerWorker() {
        while (running) {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return !running || (!readyQueue.empty() && hasIdleCore());
            });

            if (!running) break;

            while (!readyQueue.empty())
            {
                auto proc = readyQueue.front();
                readyQueue.pop();

                int core = -1;

                {
                    std::lock_guard<std::mutex> lk(coreMutex);

                    for (int i = 0; i < numCores; ++i)
                    {
                        if (!coreStatus[i])
                        {
                            core = i;
                            coreStatus[i] = true;   // reserve immediately
                            break;
                        }
                    }

                    if (core == -1)
                    {
                        readyQueue.push(proc);  // put process back
                        break;
                    }

                    coreProcess[core] = proc;
                }

                proc->setState(Process::RUNNING);
                proc->setAssignedCore(core);

                coreCV.notify_all();
            }
        }
    }

    // worker thread, executes prints
    void coreWorker(int coreId) {
        while (running) {
            std::shared_ptr<Process> proc;

            // wait until a process is asigned
            {
                std::unique_lock<std::mutex> lock(coreMutex);
                coreCV.wait(lock, [this, coreId] {
                    return !running || (coreStatus[coreId] && coreProcess.count(coreId));
                });
                if (!running) break;
                proc = coreProcess[coreId];
            }

            // execute prints
            while (proc && !proc->isFinished()) {
                proc->executePrint(coreId);
                std::this_thread::yield();
            }

            // core is free, notify scheduler
            {
                std::lock_guard<std::mutex> lock(coreMutex);
                coreStatus[coreId] = false;
                coreProcess.erase(coreId);
            }
            cv.notify_all();   // let scheduler know a core freed up
        }
    }

    // HELPERS ---
    bool hasIdleCore() const {
        std::lock_guard<std::mutex> lock(coreMutex);

        for (bool busy : coreStatus) {
            if (!busy)
                return true;
        }

        return false;
    }

    int getIdleCore() const {
        std::lock_guard<std::mutex> lock(coreMutex);

        for (int i = 0; i < numCores; ++i) {
            if (!coreStatus[i])
                return i;
        }

        return -1;
    }

    // ATTRIBUTES NG SCHEDULER ---
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