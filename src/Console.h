// ============================================================================
// Console.h — CLI Console & Command Dispatcher
// ============================================================================
// MO1 REQUIREMENT: Main menu console
//   Recognizes: initialize, exit, screen, scheduler-start, scheduler-stop,
//   report-util, clear
//
// MO1 REQUIREMENT: initialize gate
//   "No other commands should be recognized if the user hasn't typed
//    'initialize' first."
//
// MO1 REQUIREMENT: screen command (from main menu)
//   "screen -s <name>" — create new process, enter its screen
//   "screen -r <name>" — reattach to existing process screen
//   "screen -ls"       — list all processes + CPU utilization
// ============================================================================
#pragma once

#include <string>
#include <memory>

// Forward declarations
class Scheduler;
class ScreenManager;
class ConfigParser;
class ReportGenerator;

class Console
{
public:
    Console();
    ~Console();

    void run(); // main loop

private:
    void printHeader();
    void processCommand(const std::string& input);

    // Command handlers — each corresponds to a spec command
    void cmdInitialize();
    void cmdScreen(const std::string& args);
    void cmdSchedulerStart();
    void cmdSchedulerStop();
    void cmdReportUtil();
    void cmdClear();

    // State
    bool running    = true;
    bool initialized = false; // gate: must call 'initialize' first

    // Owned subsystem pointers
    std::unique_ptr<ConfigParser>    configParser;
    std::unique_ptr<Scheduler>       scheduler;
    // std::unique_ptr<ScreenManager>   screenManager;
    // std::unique_ptr<ReportGenerator> reportGenerator;
};
