// ============================================================================
// ScreenManager.h — Screen Multiplexer
// ============================================================================
// LESSON REFERENCE: Midterm Review — "ConsoleManager" pattern
//   The review introduces a ConsoleManager with registered console states.
//   ScreenManager serves as the process-screen variant of this pattern:
//   each process gets its own "screen" that the user can enter/exit.
//   This is analogous to the AConsole pattern where each screen has
//   its own display() and process() methods.
//
// MO1 REQUIREMENT: 'screen' command support (page 3-4)
//   "The 'screen' command emulates the screen multiplexer of Linux OS."
//
//   "screen -s <process name>" — create new process, enter its screen
//     - Console clears and 'moves' to the process screen
//     - Inside screen: 'process-smi' and 'exit'
//
//   "screen -r <process name>" — reattach to existing process screen
//     - If not found/finished: "Process <name> not found."
//
//   "screen -ls" — list all processes
//     - CPU utilization %, cores used, cores available
//     - Running processes list, Finished processes list
//
// MO1 REQUIREMENT: process-smi (inside screen)
//   "Prints simple information about the process (name, ID, current/total
//    instructions). If finished, print 'Finished!'"
//
// REFERENCE: Linux 'screen' command behavior
//   https://www.geeksforgeeks.org/screen-command-in-linux-with-examples/
// ============================================================================
#pragma once

#include "Process.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

// Forward declaration
class Scheduler;

class ScreenManager
{
public:
    ScreenManager(Scheduler* scheduler);

    // ── Screen Commands (called from Console) ────────────────────────────

    // TODO: Create a new process and enter its screen
    //   - Create a Process with the given name
    //   - Use addCommand() to populate its commandList with ICommand objects
    //   - Add it to the scheduler's ready queue
    //   - Enter the screen loop (clear console, show process screen)
    void createScreen(const std::string& processName);

    // TODO: Reattach to an existing process screen
    //   - Find process by name in the process map
    //   - If not found or finished: print "Process <name> not found."
    //   - If found: enter its screen
    void reattachScreen(const std::string& processName);

    // TODO: List all processes with CPU utilization
    //   - Show CPU utilization % (from scheduler)
    //   - Show cores used / cores available
    //   - List running processes: name, timestamp, core, progress
    //   - List finished processes: name, timestamp, "Finished"
    void listProcesses();

private:
    Scheduler* scheduler; // non-owning pointer

    // Map of process name → Process pointer for quick lookup
    // (processes are also owned by the scheduler's allProcesses list)
    std::map<std::string, std::shared_ptr<Process>> processMap;

    // TODO: The screen loop — runs when user is inside a process screen
    //   This is analogous to the AConsole pattern from the review:
    //   each screen has its own command loop.
    //   - Show process header (name, ID)
    //   - Accept commands: 'process-smi', 'exit'
    //   - 'process-smi': show process info + logs from outputLog
    //   - 'exit': return to main menu
    void enterScreenLoop(std::shared_ptr<Process> proc);

    // TODO: Display process-smi output
    //   - Process name
    //   - Process ID
    //   - Current command counter / total commands
    //   - If finished: "Finished!"
    //   - Print logs from process->getOutputLog()
    void showProcessInfo(std::shared_ptr<Process> proc);
};
