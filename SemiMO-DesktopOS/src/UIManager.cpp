#include "UIManager.h"

void UIManager::RegisterWindow(AppWindow* window)
{
    if (window)
        windows.push_back(window);
}

void UIManager::ShowWindow(const std::string& windowName)
{
    for (auto* window : windows) {
        if (window && window->isShown() == false) {
            window->show();
        }
    }
}

void UIManager::HideWindow(const std::string& windowName)
{
    for (auto* window : windows) {
        if (window && window->isShown()) {
            window->hide();
        }
    }
}

void UIManager::UpdateAllWindows()
{
    // Reserved for future window state updates
}

void UIManager::RenderAllWindows()
{
    for (auto* window : windows) {
        if (window)
            window->draw();
    }
}