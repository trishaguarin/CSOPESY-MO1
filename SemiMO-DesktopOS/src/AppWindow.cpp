// refer to the AWindow.cpp part of Display Interface Handout.
#include "AppWindow.h"

AppWindow::AppWindow(const std::string& name)
    : windowName(name), isVisible(false)
{
}

void AppWindow::show() {
    isVisible = true;
}

void AppWindow::hide() {
    isVisible = false;
}

bool AppWindow::isShown() const {
    return isVisible;
}

bool AppWindow::beginWindow() {
    if (!isVisible) return false;
    ImGui::Begin(windowName.c_str(), &isVisible);
    return true;
}

void AppWindow::endWindow() {
    ImGui::End();
}
