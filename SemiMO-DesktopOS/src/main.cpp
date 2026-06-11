#include <iostream>
#include "../include/GUIApplication.h"

int main()
{
    GUIApplication app;

    if (!app.initialize())
    {
        std::cerr << "[ERROR] Failed to initialize Application.\n";
        return -1;
    }

    app.run();
    app.shutdown();

    return 0;
}