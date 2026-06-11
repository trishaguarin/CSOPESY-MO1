#pragma once
#include <GLFW/glfw3.h>
#include "Desktop.h"

class GUIApplication
{
public:
    bool initialize();
    void run();
    void shutdown();

private:
    GLFWwindow* window   = nullptr;
    Desktop     desktop;
    bool        appRunning = true;
};