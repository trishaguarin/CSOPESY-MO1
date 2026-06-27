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
    std::ostringstream ss;

    float util = scheduler->getCpuUtilization();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();

    ss << "CPU utilization: " << std::fixed << std::setprecision(0) << util << "%\n";
    ss << "Cores used: " << coresUsed << "\n";
    ss << "Cores available: " << coresAvailable << "\n";
    ss << "--------------------------------------\n";

    auto allActive = scheduler->getRunningProcesses();
    auto finished = scheduler->getFinishedProcesses();

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

    ss << "Running processes:\n";
    if (runningSnaps.empty())
    {
        ss << "  (none)\n";
    }
    else
    {
        for (auto& s : runningSnaps)
        {
            ss << "  " << std::left << std::setw(15) << s.name
               << s.creationTimestamp << "   "
               << "Core: " << s.assignedCore << "   "
               << s.commandCounter << " / " << s.totalCommands << "\n";
        }
    }

    if (!waitingSnaps.empty())
    {
        ss << "\nSleeping processes:\n";
        for (auto& s : waitingSnaps)
        {
            ss << "  " << std::left << std::setw(15) << s.name
               << s.creationTimestamp << "   "
               << "Sleeping   "
               << s.commandCounter << " / " << s.totalCommands << "\n";
        }
    }

    ss << "\nFinished processes:\n";
    if (finished.empty())
    {
        ss << "  (none)\n";
    }
    else
    {
        for (auto& p : finished)
        {
            ss << "  " << std::left << std::setw(15) << p->getName()
               << p->getCreationTimestamp() << "   "
               << "Finished" << "   "
               << p->getTotalCommands() << " / " << p->getTotalCommands() << "\n";
        }
    }
    ss << "--------------------------------------\n";

    return ss.str();
}

void ReportGenerator::saveToFile(const std::string& filename) const
{
    std::string report = generateReport();
    std::ofstream file(filename);
    if (file.is_open())
    {
        file << report;
        file.close();
        std::cout << "Report saved to " << filename << "\n";
    }
    else
    {
        std::cerr << "Error: Could not open " << filename << "\n";
    }
}

void ReportGenerator::printToConsole() const
{
    std::cout << generateReport();
}
