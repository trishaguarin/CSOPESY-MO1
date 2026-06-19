#pragma once
#include "Process.h"

// First-Come-First-Serve scheduler
// Implements the multi-threaded approach from the lecture:

class Scheduler {
    int numCores;
    std::atomic<bool> running;
    std::queue<std::shared_ptr<Process>> readyQueue;
    std::vector<std::shared_ptr<Process>> allProcesses;
    std::vector<std::thread> coreThreads;
    std::mutex queueMutex;
    std::mutex listMutex;
};