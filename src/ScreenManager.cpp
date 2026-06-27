#include "ScreenManager.h"
#include "Scheduler.h"
#include "PrintCommand.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>

ScreenManager::ScreenManager(Scheduler* scheduler)
    : scheduler(scheduler)
{
}

void ScreenManager::createScreen(const std::string& processName)
{
    // Check if process name already exists
    if (processMap.count(processName))
    {
        std::cout << "Process '" << processName << "' already exists.\n";
        return;
    }

    // Generate a unique PID
    static int pidCounter = 0;
    pidCounter++;

    // Get config for instruction count range
    const auto& cfg = scheduler->getConfig();
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(cfg.minIns, cfg.maxIns);
    uint32_t numInstructions = dist(rng);

    auto proc = std::make_shared<Process>(pidCounter, processName);

    // Populate with PrintCommands (default behavior from spec)
    for (uint32_t i = 0; i < numInstructions; ++i)
    {
        proc->addCommand(std::make_shared<PrintCommand>());
    }

    processMap[processName] = proc;
    scheduler->addProcess(proc);

    // Enter the screen loop
    enterScreenLoop(proc);
}

void ScreenManager::reattachScreen(const std::string& processName)
{
    auto it = processMap.find(processName);
    if (it == processMap.end())
    {
        // Also try to find in scheduler
        auto proc = scheduler->findProcess(processName);
        if (!proc)
        {
            std::cout << "Process " << processName << " not found.\n";
            return;
        }
        if (proc->isFinished())
        {
            std::cout << "Process " << processName << " not found.\n";
            return;
        }
        // Found in scheduler but not in our map — register it
        processMap[processName] = proc;
        enterScreenLoop(proc);
        return;
    }

    if (it->second->isFinished())
    {
        std::cout << "Process " << processName << " not found.\n";
        return;
    }

    enterScreenLoop(it->second);
}

void ScreenManager::listProcesses()
{
    float util = scheduler->getCpuUtilization();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();

    std::cout << "CPU utilization: " << std::fixed << std::setprecision(0) << util << "%\n";
    std::cout << "Cores used: " << coresUsed << "\n";
    std::cout << "Cores available: " << coresAvailable << "\n";
    std::cout << "--------------------------------------\n";

    auto allActive = scheduler->getRunningProcesses();
    auto finished = scheduler->getFinishedProcesses();

    // Take atomic snapshot of each process to avoid TOCTOU race
    // (state and assignedCore must be read together, not separately)
    std::vector<Process::DisplaySnapshot> runningSnaps;
    std::vector<Process::DisplaySnapshot> waitingSnaps;

    for (auto& p : allActive)
    {
        auto snap = p->getDisplaySnapshot();
        if (snap.state == Process::RUNNING && snap.assignedCore >= 0)
            runningSnaps.push_back(snap);
        else if (snap.state == Process::WAITING)
            waitingSnaps.push_back(snap);
        // READY or mid-transition: skip (process is between states)
    }

    std::cout << "Running processes:\n";
    if (runningSnaps.empty())
    {
        std::cout << "  (none)\n";
    }
    else
    {
        for (auto& s : runningSnaps)
        {
            std::cout << "  " << std::left << std::setw(15) << s.name
                      << s.creationTimestamp << "   "
                      << "Core: " << s.assignedCore << "   "
                      << s.commandCounter << " / " << s.totalCommands << "\n";
        }
    }

    if (!waitingSnaps.empty())
    {
        std::cout << "\nSleeping processes:\n";
        for (auto& s : waitingSnaps)
        {
            std::cout << "  " << std::left << std::setw(15) << s.name
                      << s.creationTimestamp << "   "
                      << "Sleeping   "
                      << s.commandCounter << " / " << s.totalCommands << "\n";
        }
    }

    std::cout << "\nFinished processes:\n";
    if (finished.empty())
    {
        std::cout << "  (none)\n";
    }
    else
    {
        for (auto& p : finished)
        {
            std::cout << "  " << std::left << std::setw(15) << p->getName()
                      << p->getCreationTimestamp() << "   "
                      << "Finished" << "   "
                      << p->getTotalCommands() << " / " << p->getTotalCommands() << "\n";
        }
    }
    std::cout << "--------------------------------------\n";
}

void ScreenManager::enterScreenLoop(std::shared_ptr<Process> proc)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    showProcessInfo(proc);

    std::string cmd;
    while (true)
    {
        std::cout << "root:\\> ";
        if (!std::getline(std::cin, cmd))
            break;

        // Trim
        auto ltrim = cmd.find_first_not_of(" \t\r\n");
        auto rtrim = cmd.find_last_not_of(" \t\r\n");
        if (ltrim == std::string::npos) continue;
        cmd = cmd.substr(ltrim, rtrim - ltrim + 1);

        if (cmd == "exit")
        {
            break;
        }
        else if (cmd == "process-smi")
        {
            showProcessInfo(proc);
        }
        else
        {
            std::cout << "Unknown command in screen. Available: 'process-smi', 'exit'\n";
        }
    }
}

void ScreenManager::showProcessInfo(std::shared_ptr<Process> proc)
{
    std::cout << "\nProcess name: " << proc->getName() << "\n";
    std::cout << "ID: " << proc->getPID() << "\n";
    std::cout << "Logs:\n";

    const auto& logs = proc->getOutputLog();
    for (const auto& entry : logs)
        std::cout << entry << "\n";

    std::cout << "\n";

    if (proc->isFinished())
    {
        std::cout << "Finished!\n\n";
    }
    else
    {
        std::cout << "Current instruction line: " << proc->getCommandCounter() << "\n";
        std::cout << "Lines of code: " << proc->getTotalCommands() << "\n\n";
    }
}
