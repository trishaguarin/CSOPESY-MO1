#pragma once
#include "AppWindow.h"
#include <vector>
#include <string>

enum class ProcessState
{
    RUNNING,
    READY,
    WAITING,
    TERMINATED
};

struct ProcessInfo
{
    int pid;
    std::string name;
    ProcessState state;
    float cpuUsage;
    size_t memoryUsage; // in KB
};

class TaskManager : public AppWindow
{
public:
    TaskManager();

    void draw() override;

private:
    void drawProcessesTab();
    void drawPerformanceTab();
    void updatePerformanceData();
    void initDummyProcesses();

    ImVec4 getStateColor(ProcessState state);
    const char* getStateName(ProcessState state);

    std::vector<ProcessInfo> processes;

    // performance history buffers
    static const int HISTORY_SIZE = 100;
    float cpuHistory[HISTORY_SIZE] = {};
    float memoryHistory[HISTORY_SIZE] = {};
};
