#pragma once
#include <string>

enum class OSState
{
    Boot,
    Setup,
    Desktop
};

class UIManager
{
public:
    void Render();

private:
    void RenderBootScreen();
    void RenderSetupScreen();
    void RenderDesktop();

    OSState currentState = OSState::Boot;
    std::string currentTime = "2026-06-11";
};