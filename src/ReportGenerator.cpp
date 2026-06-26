// ============================================================================
// ReportGenerator.cpp — CPU Utilization Report Implementation
// ============================================================================
// LESSON REFERENCE: Midterm Review — "Logging"
// MO1 REQUIREMENT: report-util + csopesy-log.txt
// ============================================================================

#include "ReportGenerator.h"
#include "Scheduler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

ReportGenerator::ReportGenerator(Scheduler* scheduler)
    : scheduler(scheduler)
{
}

std::string ReportGenerator::generateReport() const
{
    // TODO: Build the report string
    //
    //   std::ostringstream ss;
    //
    //   ss << "CPU utilization: " << scheduler->getCpuUtilization() << "%\n";
    //   ss << "Cores used: " << scheduler->getCoresUsed() << "\n";
    //   ss << "Cores available: " << scheduler->getCoresAvailable() << "\n";
    //   ss << "--------------------------------------\n";
    //
    //   ss << "Running processes:\n";
    //   for (auto& p : scheduler->getRunningProcesses()) {
    //       ss << p->getName() << "  "
    //          << Process::getTimestamp() << "  "  // or stored creation time
    //          << "Core: " << p->getAssignedCore() << "  "
    //          << p->getCommandCounter() << "/" << p->getTotalCommands() << "\n";
    //   }
    //
    //   ss << "\nFinished processes:\n";
    //   for (auto& p : scheduler->getFinishedProcesses()) {
    //       ss << p->getName() << "  "
    //          << Process::getTimestamp() << "  "
    //          << "Finished  "
    //          << p->getTotalCommands() << "/" << p->getTotalCommands() << "\n";
    //   }
    //
    //   ss << "--------------------------------------\n";
    //
    //   return ss.str();

    return ""; // stub
}

void ReportGenerator::saveToFile(const std::string& filename) const
{
    // TODO: Save report to file
    //
    //   std::string report = generateReport();
    //   std::ofstream file(filename);
    //   if (file.is_open()) {
    //       file << report;
    //       file.close();
    //       std::cout << "Report saved to " << filename << "\n";
    //   } else {
    //       std::cerr << "Error: Could not open " << filename << "\n";
    //   }
}

void ReportGenerator::printToConsole() const
{
    // TODO: Print report to stdout
    //   std::cout << generateReport();
}
