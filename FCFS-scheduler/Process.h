#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <ctime>
 
class Process {
public:
    enum ProcessState {
        READY,
        RUNNING,
        FINISHED
        // may kulang pa ba here na need, "WAITING"?
    };
 
    Process(int pid, std::string name, int totalPrints)
        : pid(pid), name(name), totalPrints(totalPrints),
          printsExecuted(0), state(READY), assignedCore(-1) {}
 
    int getPID() const { return pid; }
    std::string getName() const { return name; }
    ProcessState getState() const { return state; }
    int getAssignedCore() const { return assignedCore; }
    int getPrintsExecuted() const { return printsExecuted; }
    int getTotalPrints() const { return totalPrints; }
 
    void setState(ProcessState s) { state = s; }
    void setAssignedCore(int core) { assignedCore = core; }
 
    bool isFinished() const { return printsExecuted >= totalPrints; }
 
    void executePrint(int coreId) {
       // TODO: to display the message on screen_01
    }

private:
    int pid;
    std::string name;
    int totalPrints;
    int printsExecuted;
    ProcessState state;
    int assignedCore;
    std::mutex fileMutex;  // protects file writes since multiple cores could finish near the same time
};