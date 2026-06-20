#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include "Process.h"
#include "Scheduler.h"

// CONSTANTS ---
const int NUM_CORES          = 4;
const int NUM_PROCESSES      = 10;
const int PRINTS_PER_PROCESS = 100;

// HELPER FUNCS ---

// for process creation timestamps
std::map<std::string, std::string> processCreationTime;
std::string nowTimestamp() {
    return Process::getTimestamp();
}

void printHeader() { // header for UI
    std::cout << R"(

	 _______  _______  _______  _______  _______  _______  __   __ 
	|       ||       ||       ||       ||       ||       ||  | |  |
	|       ||  _____||   _   ||    _  ||    ___||  _____||  |_|  |
	|       || |_____ |  | |  ||   |_| ||   |___ | |_____ |       |
	|      _||_____  ||  |_|  ||    ___||    ___||_____  ||_     _|
	|     |_  _____| ||       ||   |    |   |___  _____| |  |   |  
	|_______||_______||_______||___|    |_______||_______|  |___|  

    )" << std::endl;
    std::cout << "Type 'screen -ls' to list processes, 'exit' to quit.\n";
    std::cout << "------------------------------------------------------------\n\n";
}

// UI printer
void printProcessList(Scheduler& scheduler) {
    auto running  = scheduler.getRunningProcesses();
    auto finished = scheduler.getFinishedProcesses();

    std::cout << "\n------------------------------------------------------------\n";

    // RUNNING
    std::cout << "Running processes:\n";
    if (running.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (auto& p : running) {   
            std::string ts = processCreationTime.count(p->getName())
                             ? processCreationTime[p->getName()]
                             : nowTimestamp();

            std::cout << std::left
                      << "  " << std::setw(12) << p->getName()
                      << "  " << std::setw(22) << ts
                      << "  Core: " << p->getAssignedCore()
                      << "    " << p->getPrintsExecuted()
                      << " / " << p->getTotalPrints()
                      << "\n";
        }
    }

    std::cout << "\n";

    // FINISHED
    std::cout << "Finished processes:\n";
    if (finished.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (auto& p : finished) {
            std::string ts = processCreationTime.count(p->getName())
                             ? processCreationTime[p->getName()]
                             : nowTimestamp();

            std::cout << std::left
                      << "  " << std::setw(12) << p->getName()
                      << "  " << std::setw(22) << ts
                      << "  Finished"
                      << "    " << p->getTotalPrints()
                      << " / " << p->getTotalPrints()
                      << "\n";
        }
    }

    std::cout << "------------------------------------------------------------\n\n";
}

// ── Main ─────────────────────────────────────────────────────────────────────
int main() {
    printHeader();

    Scheduler scheduler(NUM_CORES);

    // CREATING PROCESSES ---
    std::cout << "Initialising " << NUM_PROCESSES << " processes...\n\n";
    for (int i = 1; i <= NUM_PROCESSES; ++i) {
        // Name: process01, process02, …, process10
        std::ostringstream oss;
        oss << "process" << std::setw(2) << std::setfill('0') << i;
        std::string name = oss.str();

        processCreationTime[name] = nowTimestamp();
        { std::ofstream clearFile(name + ".txt", std::ios::trunc); } // clear file -> added this line
        auto proc = std::make_shared<Process>(i, name, PRINTS_PER_PROCESS);
        scheduler.addProcess(proc);
    }

    // STARTING SCHEDULER ---
    scheduler.start();
    std::cout << "Scheduler started with " << NUM_CORES << " cores.\n";
    std::cout << "All " << NUM_PROCESSES << " processes queued.\n\n";

    // MAIN PROGRAM LOOP ---
    std::string input;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break; 

        auto trim = [](std::string s) {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
            s.erase(s.find_last_not_of(" \t\r\n") + 1);
            return s;
        };
        input = trim(input);

        if (input == "screen -ls") {
            printProcessList(scheduler);
        } else if (input == "exit") {
            std::cout << "Stopping scheduler...\n";
            scheduler.stop();
            std::cout << "Goodbye!\n";
            break;
        } else if (!input.empty()) {
            std::cout << "Unknown command: '" << input << "'. "
                         "Try 'screen -ls' or 'exit'.\n";
        }
    }

    return 0;
}