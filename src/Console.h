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
    bool running     = true;
    bool initialized = false;

    // Owned subsystem pointers
    std::unique_ptr<ConfigParser>    configParser;
    std::unique_ptr<Scheduler>       scheduler;
    std::unique_ptr<ScreenManager>   screenManager;
    std::unique_ptr<ReportGenerator> reportGenerator;
};
