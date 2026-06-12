#include "Taskbar.h"
#include "imgui.h"
#include <chrono>
#include <ctime>

void Taskbar::draw(TaskManager& taskMgr, UniqueApp1& app1, UniqueApp2& app2)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    float taskbarHeight = 48.0f;

    // position at bottom of screen
    ImGui::SetNextWindowPos(ImVec2(0, screenSize.y - taskbarHeight));
    ImGui::SetNextWindowSize(ImVec2(screenSize.x, taskbarHeight));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(20, 20, 35, 220));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

    ImGui::Begin("##Taskbar", nullptr,
        ImGuiWindowFlags_NoTitleBar   |
        ImGuiWindowFlags_NoResize     |
        ImGuiWindowFlags_NoMove       |
        ImGuiWindowFlags_NoScrollbar  |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    float btnH = 32.0f;
    float btnW = 64.0f;

    // icons
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(50, 50, 80, 200));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(70, 70, 120, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(90, 90, 150, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

    // button 1: calculator
    if (ImGui::Button("CALC", ImVec2(btnW, btnH)))
    {
        if (app1.isShown()) app1.hide();
        else app1.show();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Calculator");

    ImGui::SameLine();

    // button 2: notes
    if (ImGui::Button("NOTE", ImVec2(btnW, btnH)))
    {
        if (app2.isShown()) app2.hide();
        else app2.show();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Notepad");

    ImGui::SameLine();

    // button 3: task manager
    if (ImGui::Button("TASK", ImVec2(btnW, btnH)))
    {
        if (taskMgr.isShown()) taskMgr.hide();
        else taskMgr.show();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Task Manager");

    ImGui::PopStyleVar(); 
    ImGui::PopStyleColor(3); // button colors
    
    // system tray on the right side
    drawSystemTray();

    ImGui::End();

    ImGui::PopStyleVar();  // WindowPadding
    ImGui::PopStyleColor(); // WindowBg
}

// right-side system tray area — clock and framerate
void Taskbar::drawSystemTray()
{
    ImGuiIO& io = ImGui::GetIO();

    // get current time
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);

    char timeBuf[16];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", localTime);

    // position to right side of taskbar
    float trayWidth = 180.0f;
    ImGui::SameLine(ImGui::GetWindowWidth() - trayWidth);

    ImGui::Text("FPS: %.0f", io.Framerate);
    ImGui::SameLine();
    ImGui::Text("|  %s", timeBuf);
}
