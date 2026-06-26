// ============================================================================
// ReportGenerator.h — CPU Utilization Report
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Logging"
//   "We want to keep track of which programs use how much and what
//    kinds of computer resources."
//   "Basic logging must be supported. E.g. Viewing of running processes,
//    checking resource availability, CPU and memory utilization."
//
// MO1 REQUIREMENT: report-util (page 3-4)
//   "The console should be able to generate a utilization report whenever
//    the 'report-util' command is entered."
//
// MO1 REQUIREMENT: csopesy-log.txt (page 4)
//   "The 'report-util' command saves this into a text file 'csopesy-log.txt'."
//   "The 'screen -ls' and 'report-util' commands should be similar.
//    The only difference is that report-util saves this into a text file."
//
// Output format (from spec mockup, page 4):
//   CPU utilization: XX%
//   Cores used: X
//   Cores available: X
//   -----------------------------------------
//   Running processes:
//   <name>  <timestamp>  Core: X  <commandCounter>/<totalCommands>
//   ...
//   Finished processes:
//   <name>  <timestamp>  Finished  <totalCommands>/<totalCommands>
//   ...
//   -----------------------------------------
// ============================================================================
#pragma once

#include <string>

// Forward declaration
class Scheduler;

class ReportGenerator
{
public:
    ReportGenerator(Scheduler* scheduler);

    // TODO: Generate the report string
    //   - Same content as screen -ls
    //   - CPU utilization %, cores used, cores available
    //   - Running and finished process lists
    std::string generateReport() const;

    // TODO: Save report to csopesy-log.txt
    //   - Call generateReport()
    //   - Write to "csopesy-log.txt" in the current directory
    //   - Print confirmation to console
    void saveToFile(const std::string& filename = "csopesy-log.txt") const;

    // TODO: Print report to console (used by screen -ls)
    void printToConsole() const;

private:
    Scheduler* scheduler; // non-owning pointer
};
