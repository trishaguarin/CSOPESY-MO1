#pragma once

#include <string>

// Forward declaration
class Scheduler;

class ReportGenerator
{
public:
    ReportGenerator(Scheduler* scheduler);
    std::string generateReport() const;

    void saveToFile(const std::string& filename = "csopesy-log.txt") const;
    void printToConsole() const;

private:
    Scheduler* scheduler; // non-owning pointer
};
