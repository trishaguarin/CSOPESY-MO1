#include "ScreenManager.h"
#include "Scheduler.h"
#include "PrintCommand.h"
#include "IMemoryAllocator.h"
#include "AddCommand.h"
#include "DeclareCommand.h"
#include "ReadCommand.h"
#include "WriteCommand.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>
#include <algorithm>

ScreenManager::ScreenManager(Scheduler* scheduler)
    : scheduler(scheduler)
{
}

void ScreenManager::createScreen(const std::string& processName, size_t memorySize)
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

    auto proc = std::make_shared<Process>(pidCounter, processName, memorySize);

    // Initialize variable "x" to 0
    proc->getSymbolTable().setVariable("x", 0);

    std::uniform_int_distribution<int> addDist(1, 10);
    std::uniform_int_distribution<uint32_t> addrDist(0, static_cast<uint32_t>(memorySize > 2 ? memorySize - 2 : 0));

    for (uint32_t i = 0; i < numInstructions; ++i)
    {
        int choice = i % 4;
        switch (choice)
        {
        case 0:
            proc->addCommand(std::make_shared<PrintCommand>("Value from: ", "x"));
            break;
        case 1:
            proc->addCommand(std::make_shared<AddCommand>("x", "x", std::to_string(addDist(rng))));
            break;
        case 2:
        {
            uint32_t addr = addrDist(rng);
            addr = addr & ~1u;
            proc->addCommand(std::make_shared<WriteCommand>(addr, "x"));
            break;
        }
        case 3:
        {
            uint32_t addr = addrDist(rng);
            addr = addr & ~1u;
            proc->addCommand(std::make_shared<ReadCommand>("x", addr));
            break;
        }
        }
    }

    processMap[processName] = proc;
    scheduler->addProcess(proc);

    // Enter the screen loop
    enterScreenLoop(proc);
}

void ScreenManager::createScreenWithInstructions(const std::string& processName, size_t memorySize,
                                                   const std::string& instructionString)
{
    if (processMap.count(processName))
    {
        std::cout << "Process '" << processName << "' already exists.\n";
        return;
    }

    static int pidCounter = 0;
    pidCounter++;

    auto proc = std::make_shared<Process>(pidCounter, processName, memorySize);

    parseAndAddInstructions(proc, instructionString);

    // Validate instruction count (1-50 per spec)
    if (proc->getTotalCommands() < 1 || proc->getTotalCommands() > 50)
    {
        std::cout << "invalid command\n";
        return;
    }

    processMap[processName] = proc;
    scheduler->addProcess(proc);

    enterScreenLoop(proc);
}

void ScreenManager::parseAndAddInstructions(std::shared_ptr<Process> proc, const std::string& instructionString)
{
    // Parse semicolon-separated instructions
    std::istringstream stream(instructionString);
    std::string instruction;

    while (std::getline(stream, instruction, ';'))
    {
        // Trim whitespace
        auto ltrim = instruction.find_first_not_of(" \t\r\n");
        auto rtrim = instruction.find_last_not_of(" \t\r\n");
        if (ltrim == std::string::npos) continue;
        instruction = instruction.substr(ltrim, rtrim - ltrim + 1);

        if (instruction.empty()) continue;

        // Parse instruction type
        std::istringstream iss(instruction);
        std::string keyword;
        iss >> keyword;

        if (keyword == "PRINT")
        {
            // PRINT("message" + varName) or PRINT("message")
            // Extract the content between PRINT( and )
            size_t openParen = instruction.find('(');
            size_t closeParen = instruction.rfind(')');
            if (openParen != std::string::npos && closeParen != std::string::npos && closeParen > openParen)
            {
                std::string content = instruction.substr(openParen + 1, closeParen - openParen - 1);

                // Check for + concatenation
                size_t plusPos = content.find('+');
                if (plusPos != std::string::npos)
                {
                    std::string msgPart = content.substr(0, plusPos);
                    std::string varPart = content.substr(plusPos + 1);

                    // Trim and remove quotes from message part
                    auto ml = msgPart.find_first_not_of(" \t\"\\");
                    auto mr = msgPart.find_last_not_of(" \t\"\\");
                    std::string msg = (ml != std::string::npos) ? msgPart.substr(ml, mr - ml + 1) : "";

                    // Trim variable name
                    auto vl = varPart.find_first_not_of(" \t");
                    auto vr = varPart.find_last_not_of(" \t");
                    std::string var = (vl != std::string::npos) ? varPart.substr(vl, vr - vl + 1) : "";

                    proc->addCommand(std::make_shared<PrintCommand>(msg, var));
                }
                else
                {
                    // Just a message
                    auto ml = content.find_first_not_of(" \t\"\\");
                    auto mr = content.find_last_not_of(" \t\"\\");
                    std::string msg = (ml != std::string::npos) ? content.substr(ml, mr - ml + 1) : "";
                    proc->addCommand(std::make_shared<PrintCommand>(msg, ""));
                }
            }
        }
        else if (keyword == "DECLARE")
        {
            std::string varName;
            int value = 0;
            iss >> varName >> value;
            proc->addCommand(std::make_shared<DeclareCommand>(varName, value));
        }
        else if (keyword == "ADD")
        {
            std::string dest, src1, src2;
            iss >> dest >> src1 >> src2;
            proc->addCommand(std::make_shared<AddCommand>(dest, src1, src2));
        }
        else if (keyword == "WRITE")
        {
            // WRITE 0xADDR value_or_var
            std::string addrStr, valStr;
            iss >> addrStr >> valStr;
            uint32_t addr = 0;
            try
            {
                addr = static_cast<uint32_t>(std::stoul(addrStr, nullptr, 16));
            }
            catch (...) {}
            proc->addCommand(std::make_shared<WriteCommand>(addr, valStr));
        }
        else if (keyword == "READ")
        {
            // READ varName 0xADDR
            std::string varName, addrStr;
            iss >> varName >> addrStr;
            uint32_t addr = 0;
            try
            {
                addr = static_cast<uint32_t>(std::stoul(addrStr, nullptr, 16));
            }
            catch (...) {}
            proc->addCommand(std::make_shared<ReadCommand>(varName, addr));
        }
    }
}

void ScreenManager::reattachScreen(const std::string& processName)
{
    auto it = processMap.find(processName);
    std::shared_ptr<Process> proc;

    if (it != processMap.end())
        proc = it->second;
    else
    {
        proc = scheduler->findProcess(processName);
        if (proc)
            processMap[processName] = proc;
    }

    if (!proc)
    {
        std::cout << "Process " << processName << " not found.\n";
        return;
    }

    // Check for memory access violation (TERMINATED state)
    if (proc->isTerminated())
    {
        std::cout << "Process " << processName
                  << " shut down due to memory access violation error that occurred at "
                  << proc->getViolationTime() << ". "
                  << proc->getViolationAddress() << " invalid.\n";
        return;
    }

    if (proc->getState() == Process::FINISHED)
    {
        std::cout << "Process " << processName << " not found.\n";
        return;
    }

    enterScreenLoop(proc);
}

void ScreenManager::listProcesses()
{
    float util = scheduler->getCpuUtilization();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();

    std::cout << "CPU utilization: " << std::fixed << std::setprecision(0) << util << "%\n";
    std::cout << "Cores used: " << coresUsed << "\n";
    std::cout << "Cores available: " << coresAvailable << "\n";

    auto* memAlloc = scheduler->getMemoryAllocator();
    if (memAlloc)
    {
        uint32_t usedMem = memAlloc->getUsedMemory();
        uint32_t totalMem = memAlloc->getTotalMemory();
        uint32_t extFrag = memAlloc->getExternalFragmentation();
        std::cout << "Memory Usage: " << usedMem << " / " << totalMem << "\n";
        std::cout << "External Fragmentation: " << extFrag << "\n";
        std::cout << "Pages paged in: " << memAlloc->getNumPagedIn() << "\n";
        std::cout << "Pages paged out: " << memAlloc->getNumPagedOut() << "\n";
    }

    std::cout << "--------------------------------------\n";

    auto allActive = scheduler->getRunningProcesses();
    auto finished = scheduler->getFinishedProcesses();

    // Take atomic snapshot of each process to avoid TOCTOU race
    std::vector<Process::DisplaySnapshot> runningSnaps;
    std::vector<Process::DisplaySnapshot> waitingSnaps;

    for (auto& p : allActive)
    {
        auto snap = p->getDisplaySnapshot();
        if (snap.state == Process::RUNNING && snap.assignedCore >= 0)
            runningSnaps.push_back(snap);
        else if (snap.state == Process::WAITING)
            waitingSnaps.push_back(snap);
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

void ScreenManager::printProcessSmi()
{
    auto* memAlloc = scheduler->getMemoryAllocator();
    if (!memAlloc)
    {
        std::cout << "Memory allocator not initialized.\n";
        return;
    }

    std::cout << "----------------------------------------------\n";
    std::cout << "| PROCESS-SMI V01.00 Driver Version: 01.00  |\n";
    std::cout << "----------------------------------------------\n";

    float util = scheduler->getCpuUtilization();
    uint32_t usedMem = memAlloc->getUsedMemory();
    uint32_t totalMem = memAlloc->getTotalMemory();

    std::cout << " CPU-Util: " << std::fixed << std::setprecision(0) << util << "%\n";
    std::cout << " Memory Usage: " << usedMem << "MiB / " << totalMem << "MiB\n";
    std::cout << "----------------------------------------------\n";

    // Process list with memory
    std::cout << "==============================================\n";
    std::cout << " Running processes and memory usage:\n";
    std::cout << "----------------------------------------------\n";

    auto allProcs = scheduler->getAllProcesses();
    bool anyShown = false;
    for (auto& p : allProcs)
    {
        auto s = p->getState();
        if (s == Process::RUNNING || s == Process::READY || s == Process::WAITING)
        {
            size_t procMem = memAlloc->getProcessMemorySize(p->getName());
            if (procMem > 0 || memAlloc->hasAllocation(p->getName()))
            {
                std::cout << " " << std::left << std::setw(20) << p->getName()
                          << procMem << "MiB\n";
                anyShown = true;
            }
        }
    }
    if (!anyShown)
        std::cout << " (none)\n";

    std::cout << "----------------------------------------------\n";
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

    if (proc->isTerminated())
    {
        std::cout << "Status: TERMINATED (memory access violation at "
                  << proc->getViolationTime() << ", address " << proc->getViolationAddress() << ")\n\n";
        return;
    }

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
