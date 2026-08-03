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

    void createScreen(const std::string& processName, size_t memorySize);
    void createScreenWithInstructions(const std::string& processName, size_t memorySize,
                                       const std::string& instructionString);
    void reattachScreen(const std::string& processName);
    void listProcesses();
    void printProcessSmi();

private:
    Scheduler* scheduler; // non-owning pointer

    std::map<std::string, std::shared_ptr<Process>> processMap;

    void enterScreenLoop(std::shared_ptr<Process> proc);
    void showProcessInfo(std::shared_ptr<Process> proc);
    void parseAndAddInstructions(std::shared_ptr<Process> proc, const std::string& instructionString);
};
