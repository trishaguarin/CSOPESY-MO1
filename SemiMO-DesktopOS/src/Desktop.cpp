//refer to the Desktop.cpp part of Display Interface Handout

#include "Desktop.h"
#include "Taskbar.h"
#include <chrono> 


void Desktop::draw() {
    //type niyo naang asa handout
}

void Desktop::drawClock () {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);

    //pakicontinue nlanag
}

void Desktop::powerButton() {
    //pakicontinue nalang    
}


//smth add the taskbar here as well to incorporate it

