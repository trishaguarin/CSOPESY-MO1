#include "Console.h"
#include "ConfigParser.h"
#include "Scheduler.h"
#include "ScreenManager.h"
#include "ReportGenerator.h"
#include "IMemoryAllocator.h"

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

Console::Console() = default;

Console::~Console()
{
    if (scheduler)
        scheduler->stop();
}

void Console::printHeader()
{
    std::cout << R"(

	 _______  _______  _______  _______  _______  _______  __   __ 
	|       ||       ||       ||       ||       ||       ||  | |  |
	|       ||  _____||   _   ||    _  ||    ___||  _____||  |_|  |
	|       || |_____ |  | |  ||   |_| ||   |___ | |_____ |       |
	|      _||_____  ||  |_|  ||    ___||    ___||_____  ||_     _|
	|     |_  _____| ||       ||   |    |   |___  _____| |  |   |  
	|_______||_______||_______||___|    |_______||_______|  |___|  

    )" << std::endl;

    std::cout << "Hello, welcome to the CSOPESY command line!" << std::endl;
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen" << std::endl;
    std::cout << "\n** IMPORTANT: Type 'initialize' to load config and start system **\n" << std::endl;
}

void Console::run()
{
    printHeader();

    std::string input;
    while (running)
    {
        std::cout << "> ";
        if (!std::getline(std::cin, input))
            break;

        // Trim whitespace
        auto ltrim = input.find_first_not_of(" \t\r\n");
        auto rtrim = input.find_last_not_of(" \t\r\n");
        if (ltrim == std::string::npos)
            continue;
        input = input.substr(ltrim, rtrim - ltrim + 1);

        if (input.empty())
            continue;

        processCommand(input);
    }
}

void Console::processCommand(const std::string& input)
{
    if (input == "exit")
    {
        running = false;
        std::cout << "Goodbye!\n";
        return;
    }

    if (input == "initialize")
    {
        cmdInitialize();
        return;
    }

    // Gate: everything below requires initialize
    if (!initialized)
    {
        std::cout << "Error: Please run 'initialize' first.\n";
        return;
    }

    if (input == "clear")
    {
        cmdClear();
        return;
    }

    // Parse 'screen' with subcommands
    if (input.substr(0, 6) == "screen")
    {
        std::string args = (input.size() > 7) ? input.substr(7) : "";
        cmdScreen(args);
        return;
    }

    if (input == "scheduler-test" || input == "scheduler-start")
    {
        cmdSchedulerStart();
        return;
    }

    if (input == "scheduler-stop")
    {
        cmdSchedulerStop();
        return;
    }

    if (input == "report-util")
    {
        cmdReportUtil();
        return;
    }

    if (input == "process-smi")
    {
        cmdProcessSmi();
        return;
    }

    if (input == "vmstat")
    {
        cmdVmstat();
        return;
    }

    std::cout << "Unknown command: '" << input << "'\n";
}

void Console::cmdInitialize()
{
    if (initialized)
    {
        std::cout << "System already initialized.\n";
        return;
    }

    configParser = std::make_unique<ConfigParser>();
    if (!configParser->loadFromFile("config.txt"))
    {
        std::cerr << "Error: Failed to load config.txt. Please ensure it exists.\n";
        configParser.reset();
        return;
    }

    auto cfg = configParser->getConfig();
    scheduler       = std::make_unique<Scheduler>(cfg);
    screenManager   = std::make_unique<ScreenManager>(scheduler.get());
    reportGenerator = std::make_unique<ReportGenerator>(scheduler.get());

    scheduler->start();

    initialized = true;
    std::cout << "System initialized.\n";
}

void Console::cmdScreen(const std::string& args)
{
    if (args.empty())
    {
        std::cout << "Usage: screen -s <name> <memsize> | screen -c <name> <memsize> \"<instructions>\" | screen -r <name> | screen -ls\n";
        return;
    }

    if (args == "-ls")
    {
        screenManager->listProcesses();
        return;
    }

    // screen -s <name> <memsize>
    if (args.size() > 3 && args.substr(0, 2) == "-s" && args[2] == ' ')
    {
        std::string remainder = args.substr(3);
        std::istringstream iss(remainder);
        std::string processName;
        size_t memSize = 0;
        iss >> processName >> memSize;

        if (processName.empty())
        {
            std::cout << "Error: Process name cannot be empty.\n";
            return;
        }

        if (memSize == 0)
        {
            // Default to minMemPerProc if not specified
            memSize = scheduler->getConfig().minMemPerProc;
        }

        if (!isValidMemorySize(memSize))
        {
            std::cout << "invalid memory allocation\n";
            return;
        }

        screenManager->createScreen(processName, memSize);
        printHeader();
        return;
    }

    // screen -c <name> [memsize] "<instructions>"
    if (args.size() > 3 && args.substr(0, 2) == "-c" && args[2] == ' ')
    {
        std::string remainder = args.substr(3);

        auto lt = remainder.find_first_not_of(" \t");
        if (lt != std::string::npos) remainder = remainder.substr(lt);

        std::istringstream iss(remainder);
        std::string processName;
        iss >> processName;

        if (processName.empty())
        {
            std::cout << "Error: Process name cannot be empty.\n";
            return;
        }

        size_t nameEnd = remainder.find(processName) + processName.length();
        std::string rest = remainder.substr(nameEnd);
        auto restLt = rest.find_first_not_of(" \t");
        if (restLt != std::string::npos) rest = rest.substr(restLt);
        else rest = "";

        size_t memSize = 0;
        if (!rest.empty() && std::isdigit(static_cast<unsigned char>(rest[0])))
        {
            std::istringstream restIss(rest);
            restIss >> memSize;
        }

        if (memSize == 0)
        {
            memSize = scheduler->getConfig().minMemPerProc;
        }

        if (!isValidMemorySize(memSize))
        {
            std::cout << "invalid memory allocation\n";
            return;
        }

        // Extract instructions between quotes
        size_t firstQuote = remainder.find('"');
        size_t lastQuote = remainder.rfind('"');
        if (firstQuote == std::string::npos || firstQuote == lastQuote)
        {
            std::cout << "Error: Instructions must be enclosed in quotes.\n";
            return;
        }

        std::string instructions = remainder.substr(firstQuote + 1, lastQuote - firstQuote - 1);

        screenManager->createScreenWithInstructions(processName, memSize, instructions);
        printHeader();
        return;
    }

    // screen -r <name>
    if (args.size() > 3 && args.substr(0, 2) == "-r" && args[2] == ' ')
    {
        std::string processName = args.substr(3);
        auto lt = processName.find_first_not_of(" \t");
        auto rt = processName.find_last_not_of(" \t");
        if (lt == std::string::npos)
        {
            std::cout << "Error: Process name cannot be empty.\n";
            return;
        }
        processName = processName.substr(lt, rt - lt + 1);
        screenManager->reattachScreen(processName);
        printHeader();
        return;
    }

    std::cout << "Unknown screen subcommand: '" << args << "'\n";
}

void Console::cmdSchedulerStart()
{
    scheduler->startBatchGeneration();
    std::cout << "Batch process generation started.\n";
}

void Console::cmdSchedulerStop()
{
    scheduler->stopBatchGeneration();
    std::cout << "Batch process generation stopped.\n";
}

void Console::cmdReportUtil()
{
    reportGenerator->printToConsole();
    reportGenerator->saveToFile("csopesy-log.txt");
}

void Console::cmdProcessSmi()
{
    screenManager->printProcessSmi();
}

void Console::cmdVmstat()
{
    auto* memAlloc = scheduler->getMemoryAllocator();
    if (!memAlloc)
    {
        std::cout << "Memory allocator not initialized.\n";
        return;
    }

    uint32_t totalMem = memAlloc->getTotalMemory();
    uint32_t usedMem = memAlloc->getUsedMemory();
    uint32_t freeMem = totalMem - usedMem;

    uint64_t idleTicks = scheduler->getIdleCpuTicks();
    uint64_t activeTicks = scheduler->getActiveCpuTicks();
    uint64_t totalTicks = scheduler->getTotalCpuTicks();

    uint64_t pagedIn = memAlloc->getNumPagedIn();
    uint64_t pagedOut = memAlloc->getNumPagedOut();

    std::cout << std::right << std::setw(10) << totalMem   << " Total memory\n";
    std::cout << std::right << std::setw(10) << usedMem    << " Used memory\n";
    std::cout << std::right << std::setw(10) << freeMem    << " Free memory\n";
    std::cout << std::right << std::setw(10) << idleTicks  << " Idle cpu ticks\n";
    std::cout << std::right << std::setw(10) << activeTicks<< " Active cpu ticks\n";
    std::cout << std::right << std::setw(10) << totalTicks << " Total cpu ticks\n";
    std::cout << std::right << std::setw(10) << pagedIn    << " Num paged in\n";
    std::cout << std::right << std::setw(10) << pagedOut   << " Num paged out\n";
}

void Console::cmdClear()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printHeader();
}

bool Console::isValidMemorySize(size_t size) const
{
    // Must be power of 2, in range [64, 65536]
    if (size < 64 || size > 65536)
        return false;
    // Check power of 2
    return (size & (size - 1)) == 0;
}
