#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <chrono>

class Process {
public:
    enum ProcessState {
        READY,
        RUNNING,
        FINISHED
        // "WAITING"?
    };

    Process(int pid, std::string name, int totalPrints)
        : pid(pid), name(name), totalPrints(totalPrints),
          printsExecuted(0), state(READY), assignedCore(-1) {

        auto now = std::chrono::system_clock::now();
        creationTime = std::chrono::system_clock::to_time_t(now);
    }

    int getPID() const { return pid; }
    std::string getName() const { return name; }
    ProcessState getState() const { return state.load(); }
    int getAssignedCore() const { return assignedCore.load(); }
    int getPrintsExecuted() const { return printsExecuted.load(); }
    int getTotalPrints() const { return totalPrints; }
    std::time_t getCreationTime() const { return creationTime; }

    void setState(ProcessState s) { state.store(s); }
    void setAssignedCore(int core) { assignedCore.store(core); }

    bool isFinished() const { return printsExecuted.load() >= totalPrints; }

    static std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_info;
#ifdef _WIN32
        localtime_s(&tm_info, &t);
#else
        localtime_r(&t, &tm_info);
#endif
        std::ostringstream oss;
        int hour = tm_info.tm_hour;
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        if (hour == 0)  hour = 12;
        else if (hour > 12) hour -= 12;

        oss << "(" 
            << std::setw(2) << std::setfill('0') << (tm_info.tm_mon + 1) << "/"
            << std::setw(2) << std::setfill('0') << tm_info.tm_mday << "/"
            << (tm_info.tm_year + 1900) << " "
            << std::setw(2) << std::setfill('0') << hour << ":"
            << std::setw(2) << std::setfill('0') << tm_info.tm_min << ":"
            << std::setw(2) << std::setfill('0') << tm_info.tm_sec
            << ampm << ")";
        return oss.str();
    }
    
    void executePrint(int coreId) {
        std::string timestamp = getTimestamp();
        std::lock_guard<std::mutex> lock(fileMutex); //pinagbaliktad ko ^^

        // FIXME: change for MP submission, comment out
        {
            std::ofstream outFile(name + ".txt", std::ios::app);
            if (outFile.is_open()) {
                outFile << timestamp
                        << "    Core:" << coreId
                        << "    \"Hello world from " << name << "!\"\n";
            }
        }

        printsExecuted++;

        if (printsExecuted.load() >= totalPrints) {
            state.store(FINISHED);
        }
    }

private:
    int pid;
    std::string name;
    int totalPrints;
    std::atomic<int> printsExecuted;
    std::atomic<ProcessState> state;
    std::atomic<int> assignedCore;
    std::time_t creationTime;
    std::mutex fileMutex;  // protects file writes
};