#pragma once
#include <string>
#include <vector>
#include "AppWindow.h"

class UIManager
{
public:
    // Window registry management
    void RegisterWindow(AppWindow* window);
    void ShowWindow(const std::string& windowName);
    void HideWindow(const std::string& windowName);
    
    // Update and render all managed windows
    void UpdateAllWindows();
    void RenderAllWindows();

private:
    std::vector<AppWindow*> windows;
};