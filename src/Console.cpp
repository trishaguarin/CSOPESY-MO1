// ============================================================================
// Console.cpp — CLI Console Implementation
// ============================================================================
// MO1 REQUIREMENT: Main menu console
//   The main loop reads user input and dispatches to the correct handler.
//
// MO1 REQUIREMENT: initialize gate
//   All commands except 'exit' should print an error if !initialized.
//
// REFERENCE: Week2GroupHW.cpp had the skeleton for this.
//   Key differences from the old code:
//   - Commands now have real logic behind them (not just "recognized")
//   - 'screen' must be parsed for subcommands (-s, -r, -ls)
//   - 'initialize' must read config.txt before anything works
// ============================================================================

#include "Console.h"
#include "ConfigParser.h"
// #include "Scheduler.h"
// #include "ScreenManager.h"
// #include "ReportGenerator.h"

#include <iostream>
#include <string>
#include <algorithm>

Console::Console()
{
    configParser = std::make_unique<ConfigParser>();
}

Console::~Console() = default;

// ── ASCII Header ─────────────────────────────────────────────────────────────
// MO1 REQUIREMENT: "A main menu console" — present a branded CLI
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

// ── Main Loop ────────────────────────────────────────────────────────────────
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
            continue; // empty input
        input = input.substr(ltrim, rtrim - ltrim + 1);

        if (input.empty())
            continue;

        processCommand(input);
    }
}

// ── Command Dispatcher ───────────────────────────────────────────────────────
// MO1 REQUIREMENT: recognize initialize, exit, screen, scheduler-start,
//                  scheduler-stop, report-util
void Console::processCommand(const std::string& input)
{
    // 'exit' always works, even before initialize
    if (input == "exit")
    {
        running = false;
        std::cout << "Goodbye!\n";
        return;
    }

    // 'clear' always works
    if (input == "clear")
    {
        cmdClear();
        return;
    }

    // 'initialize' — boot the system
    if (input == "initialize")
    {
        cmdInitialize();
        return;
    }

    // ── Gate: everything below requires initialize ──
    if (!initialized)
    {
        std::cout << "Error: Please run 'initialize' first.\n";
        return;
    }

    // Parse 'screen' with subcommands
    if (input.substr(0, 6) == "screen")
    {
        std::string args = (input.size() > 7) ? input.substr(7) : "";
        cmdScreen(args);
        return;
    }

    if (input == "scheduler-start")
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

    std::cout << "Unknown command: '" << input << "'\n";
}

// ── Command Implementations ──────────────────────────────────────────────────

// MO1 REQUIREMENT: Configuration setting
//   "The 'initialize' command should read from a 'config.txt' file,
//    the parameters for your CPU scheduler and process attributes."
void Console::cmdInitialize()
{
    if (initialized)
    {
        std::cout << "System already initialized.\n";
        return;
    }

    if (!configParser)
    {
        std::cout << "Internal error: Config parser unavailable.\n";
        return;
    }

    if (!configParser->loadFromFile("config.txt"))
    {
        std::cout << "Failed to initialize system. Check config.txt for errors.\n";
        return;
    }

    const SystemConfig& config = configParser->getConfig();
    std::cout << "Configuration loaded successfully:\n";
    std::cout << "  num-cpu: " << config.numCpu << "\n";
    std::cout << "  scheduler: " << config.schedulerAlgo << "\n";
    std::cout << "  quantum-cycles: " << config.quantumCycles << "\n";
    std::cout << "  batch-process-freq: " << config.batchProcessFreq << "\n";
    std::cout << "  min-ins: " << config.minIns << "\n";
    std::cout << "  max-ins: " << config.maxIns << "\n";
    std::cout << "  delays-per-exec: " << config.delaysPerExec << "\n";

    initialized = true;
    std::cout << "System initialized.\n";
}

// MO1 REQUIREMENT: screen command support
//   "screen -s <name>" — create process + enter screen
//   "screen -r <name>" — reattach to process screen
//   "screen -ls"       — list all processes
void Console::cmdScreen(const std::string& args)
{
    if (args.empty())
    {
        std::cout << "Usage: screen -s <name> | screen -r <name> | screen -ls\n";
        return;
    }

    if (args == "-ls")
    {
        // TODO: screenManager->listProcesses();
        //   Should show: CPU utilization %, cores used, cores available,
        //   running processes list, finished processes list
        std::cout << "'screen -ls' — TODO: List all processes.\n";
        return;
    }

    if (args.substr(0, 2) == "-s" && args.size() > 3)
    {
        std::string processName = args.substr(3);
        // TODO: screenManager->createScreen(processName);
        //   - Create a new Process with the given name
        //   - Clear console, enter process screen
        //   - Inside screen: support 'process-smi' and 'exit'
        std::cout << "'screen -s " << processName << "' — TODO: Create process.\n";
        return;
    }

    if (args.substr(0, 2) == "-r" && args.size() > 3)
    {
        std::string processName = args.substr(3);
        // TODO: screenManager->reattachScreen(processName);
        //   - Find process by name
        //   - If not found or finished: "Process <name> not found."
        //   - If found: enter its screen
        std::cout << "'screen -r " << processName << "' — TODO: Reattach.\n";
        return;
    }

    std::cout << "Unknown screen subcommand: '" << args << "'\n";
}

// MO1 REQUIREMENT: scheduler-start
//   "Every X CPU ticks, a new process is generated and put into the
//    ready queue. X is set by batch-process-freq in config.txt."
void Console::cmdSchedulerStart()
{
    // TODO: Start the batch process generation loop
    //   scheduler->startBatchGeneration();
    //   - Uses batch-process-freq from config
    //   - Generates processes with randomized instructions
    //   - Instruction count between min-ins and max-ins
    //   - Process names: p01, p02, ..., p1240, etc.
    std::cout << "'scheduler-start' — TODO: Start batch generation.\n";
}

// MO1 REQUIREMENT: scheduler-stop
//   "Stops generating dummy processes."
void Console::cmdSchedulerStop()
{
    // TODO: Stop the batch process generation loop
    //   scheduler->stopBatchGeneration();
    std::cout << "'scheduler-stop' — TODO: Stop batch generation.\n";
}

// MO1 REQUIREMENT: report-util
//   "Generate a utilization report. Same as screen -ls but saves to
//    csopesy-log.txt."
void Console::cmdReportUtil()
{
    // TODO: reportGenerator->generateReport(scheduler);
    //   - CPU utilization %
    //   - Cores used / cores available
    //   - Running and finished process lists
    //   - Save to csopesy-log.txt
    std::cout << "'report-util' — TODO: Generate report.\n";
}

// Self-explanatory
void Console::cmdClear()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printHeader();
}
