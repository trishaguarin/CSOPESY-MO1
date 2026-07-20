#include "Console.h"
#include "ConfigParser.h"
#include "Scheduler.h"
#include "ScreenManager.h"
#include "ReportGenerator.h"

#include <iostream>
#include <string>

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
        std::cout << "Usage: screen -s <name> | screen -r <name> | screen -ls\n";
        return;
    }

    if (args == "-ls")
    {
        screenManager->listProcesses();
        return;
    }

    if (args.size() > 3 && args.substr(0, 2) == "-s" && args[2] == ' ')
    {
        std::string processName = args.substr(3);
        // Trim the name
        auto lt = processName.find_first_not_of(" \t");
        auto rt = processName.find_last_not_of(" \t");
        if (lt == std::string::npos)
        {
            std::cout << "Error: Process name cannot be empty.\n";
            return;
        }
        processName = processName.substr(lt, rt - lt + 1);
        screenManager->createScreen(processName);
        // After returning from screen, reprint header
        printHeader();
        return;
    }

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

    std::cout << totalMem << "\n";
    std::cout << usedMem << "\n";
    std::cout << freeMem << "\n";
    std::cout << memAlloc->getNumPagedIn() << "\n";
    std::cout << memAlloc->getNumPagedOut() << "\n";
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
