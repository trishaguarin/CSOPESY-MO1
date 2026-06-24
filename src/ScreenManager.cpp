// ============================================================================
// ScreenManager.cpp — Screen Multiplexer Implementation
// ============================================================================
// MO1 REQUIREMENT: Screen command support
//   Implements the process screen multiplexer.
//
// REFERENCE: The spec mockup (page 3) shows:
//   Line 1:  "Process name: p01"
//   Line 2:  "ID: 1024"
//   Line 3:  ""
//   Line 4:  "Current instruction line: 50"
//   Line 5:  "Lines of code: 100"
//   Lines 6+: logs from PRINT instructions
//   If finished: "Finished!"
// ============================================================================

#include "ScreenManager.h"
#include "Scheduler.h"
#include <iostream>
#include <iomanip>

ScreenManager::ScreenManager(Scheduler* scheduler)
    : scheduler(scheduler)
{
}

// MO1 REQUIREMENT: "screen -s <process name>"
//   "When the user types 'screen -s <process name>' from the main menu,
//    the console will clear its contents and 'move' to the process screen."
void ScreenManager::createScreen(const std::string& processName)
{
    // TODO: Implement screen creation
    //
    //   1. Check if a process with this name already exists
    //      if (processMap.count(processName)) {
    //          std::cout << "Process '" << processName << "' already exists.\n";
    //          return;
    //      }
    //
    //   2. Create a new Process
    //      - Generate a unique PID
    //      - Create with a default instruction set (just PRINTs for now?)
    //        Or: create with empty instructions that the scheduler will fill?
    //      auto proc = std::make_shared<Process>(pid, processName, instructions);
    //
    //   3. Register in processMap and add to scheduler
    //      processMap[processName] = proc;
    //      scheduler->addProcess(proc);
    //
    //   4. Enter the screen loop
    //      enterScreenLoop(proc);
}

// MO1 REQUIREMENT: "screen -r <process name>"
//   "The user can access the screen anytime by typing 'screen -r <name>'
//    in the main menu. If the process name is not found/finished execution,
//    the console prints 'Process <name> not found.'"
void ScreenManager::reattachScreen(const std::string& processName)
{
    // TODO: Implement screen reattach
    //
    //   1. Find the process by name
    //      auto it = processMap.find(processName);
    //
    //   2. If not found or finished:
    //      std::cout << "Process " << processName << " not found.\n";
    //      return;
    //
    //   3. If found and still running/ready:
    //      enterScreenLoop(it->second);
}

// MO1 REQUIREMENT: "screen -ls"
//   "List CPU utilization, cores used, and cores available, as well as
//    print a summary of the running and finished processes."
void ScreenManager::listProcesses()
{
    // TODO: Implement process listing
    //
    //   Spec mockup format:
    //     CPU utilization: XX%
    //     Cores used: X
    //     Cores available: X
    //     ---
    //     Running processes:
    //       <name>   <timestamp>   Core: X   <current>/<total>
    //     Finished processes:
    //       <name>   <timestamp>   Finished  <total>/<total>
    //     ---
    //
    //   Use:
    //     scheduler->getCpuUtilization()
    //     scheduler->getCoresUsed()
    //     scheduler->getCoresAvailable()
    //     scheduler->getRunningProcesses()
    //     scheduler->getFinishedProcesses()
}

// ── Screen Loop ──────────────────────────────────────────────────────────────

void ScreenManager::enterScreenLoop(std::shared_ptr<Process> proc)
{
    // TODO: Implement the screen loop
    //
    //   1. Clear console
    //      #ifdef _WIN32
    //          system("cls");
    //      #else
    //          system("clear");
    //      #endif
    //
    //   2. Show process header
    //      std::cout << "Process name: " << proc->getName() << "\n";
    //      std::cout << "ID: " << proc->getPID() << "\n\n";
    //
    //   3. Command loop
    //      while (true) {
    //          std::cout << "root:\\> ";
    //          std::string cmd;
    //          std::getline(std::cin, cmd);
    //
    //          if (cmd == "process-smi") {
    //              showProcessInfo(proc);
    //          }
    //          else if (cmd == "exit") {
    //              break; // return to main menu
    //          }
    //          else {
    //              std::cout << "Unknown command in screen.\n";
    //          }
    //      }
}

// ── Process Info Display ─────────────────────────────────────────────────────

void ScreenManager::showProcessInfo(std::shared_ptr<Process> proc)
{
    // TODO: Implement process-smi display
    //
    //   MO1 REQUIREMENT: process-smi
    //     "Prints simple information about the process. If the process has
    //      finished, simply print 'Finished!' after the process name, ID,
    //      and logs have been printed."
    //
    //   Format:
    //     Process name: <name>
    //     ID: <pid>
    //
    //     if (finished):
    //         Finished!
    //     else:
    //         Current instruction line: <currentLine>
    //         Lines of code: <totalLines>
    //
    //     <print logs from process output buffer>
}
