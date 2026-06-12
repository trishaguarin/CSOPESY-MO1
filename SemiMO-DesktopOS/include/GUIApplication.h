#pragma once
struct GLFWwindow;

#include "Desktop.h"
#include "Taskbar.h"
#include "TaskManager.h"
#include "UniqueApp1.h"
#include "UniqueApp2.h"

class GUIApplication {
public:
    bool initialize();
    void run();
    void shutdown();

private:
    GLFWwindow* window = nullptr;
    Desktop desktop;
    Taskbar taskbar;
    TaskManager taskManager;
    UniqueApp1 app1;
    UniqueApp2 app2;

    bool appRunning = true;
};
