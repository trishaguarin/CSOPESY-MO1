#pragma once

#include "Process.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

// Forward declaration
class Scheduler;

class ScreenManager
{
public:
    ScreenManager(Scheduler* scheduler);
    void createScreen(const std::string& processName);
    void reattachScreen(const std::string& processName);
    void listProcesses();

private:
    Scheduler* scheduler; // non-owning pointer

    std::map<std::string, std::shared_ptr<Process>> processMap;

    void enterScreenLoop(std::shared_ptr<Process> proc);
    void showProcessInfo(std::shared_ptr<Process> proc);
};
