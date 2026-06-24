// ============================================================================
// main.cpp — Entry Point
// ============================================================================
// MO1 REQUIREMENT: Main menu console
//   "A main menu console for recognizing the following commands:
//    initialize, exit, screen, scheduler-start, scheduler-stop, report-util"
//
// MO1 REQUIREMENT: initialize gate
//   "The 'initialize' command must be called before any other command could
//    be recognized, aside from 'exit'."
// ============================================================================

#include <iostream>
#include "Console.h"

int main()
{
    Console console;
    console.run();

    return 0;
}
