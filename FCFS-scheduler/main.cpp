#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include "Process.h"
#include "Scheduler.h"

int main() {
    // 4 cores declared for the CPU scheduler as required
    const int NUM_CORES = 4;
    const int NUM_PROCESSES = 10;
    const int PRINTS_PER_PROCESS = 100;
}