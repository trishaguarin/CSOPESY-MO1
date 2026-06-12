#include "TaskManager.h"
#include "imgui.h"
#include <cstdlib>

TaskManager::TaskManager()
    : AppWindow("Task Manager")
{
    initDummyProcesses();

    for (int i = 0; i < HISTORY_SIZE; i++)
    {
        cpuHistory[i] = 0.0f;
        memoryHistory[i] = 0.0f;
    }
}

// dummy data only !
void TaskManager::initDummyProcesses()
{
    processes = {
        {0,    "System Idle Process", ProcessState::RUNNING,    82.5f,  128},
        {4,    "System",              ProcessState::RUNNING,     0.3f,  1024},
        {512,  "svchost.exe",         ProcessState::RUNNING,     0.5f,  34816},
        {768,  "dwm.exe",             ProcessState::RUNNING,     1.5f,  65536},
        {1024, "csopesy.exe",         ProcessState::RUNNING,     4.3f,  46284},
        {2048, "explorer.exe",        ProcessState::RUNNING,     1.8f,  84684},
        {3072, "chrome.exe",          ProcessState::WAITING,     3.2f, 262963},
        {4096, "discord.exe",         ProcessState::WAITING,     1.1f, 131481},
        {5120, "notepad.exe",         ProcessState::READY,       0.0f,   8499},
        {6144, "taskmgr.exe",         ProcessState::RUNNING,     0.8f,  22630},
        {7168, "conhost.exe",         ProcessState::READY,       0.1f,  12800},
        {8192, "RuntimeBroker.exe",   ProcessState::TERMINATED,  0.0f,   4096}
    };
}

void TaskManager::draw()
{
    if (!isShown())
        return;

    ImGui::SetNextWindowSize(ImVec2(580, 420), ImGuiCond_FirstUseEver);

    if (!beginWindow())
        return;

    if (ImGui::BeginTabBar("TaskMgrTabs"))
    {
        if (ImGui::BeginTabItem("Processes"))
        {
            drawProcessesTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Performance"))
        {
            drawPerformanceTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    endWindow();
}

void TaskManager::drawProcessesTab()
{
    if (ImGui::BeginTable("ProcessTable", 5,
        ImGuiTableFlags_Sortable  |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Borders   |
        ImGuiTableFlags_RowBg     |
        ImGuiTableFlags_ScrollY,
        ImVec2(0, -30))) // summary spacing
    {
        ImGui::TableSetupColumn("PID",    ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("State");
        ImGui::TableSetupColumn("CPU %");
        ImGui::TableSetupColumn("Memory");
        ImGui::TableHeadersRow();

        for (const auto& proc : processes)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", proc.pid);

            ImGui::TableNextColumn();
            ImGui::Text("%s", proc.name.c_str());

            ImGui::TableNextColumn();
            ImVec4 stateColor = getStateColor(proc.state);
            ImGui::TextColored(stateColor, "%s", getStateName(proc.state));

            ImGui::TableNextColumn();
            ImGui::Text("%.1f%%", proc.cpuUsage);

            ImGui::TableNextColumn();
            ImGui::Text("%zu KB", proc.memoryUsage);
        }

        ImGui::EndTable();
    }

    // summary footer
    float totalCpu = 0.0f;
    size_t totalMem = 0;
    for (const auto& p : processes)
    {
        totalCpu += p.cpuUsage;
        totalMem += p.memoryUsage;
    }
    ImGui::Separator();
    ImGui::Text("Processes: %d    CPU: %.1f%%    Memory: %zu KB",
        (int)processes.size(), totalCpu, totalMem);
}

void TaskManager::drawPerformanceTab()
{
    updatePerformanceData();

    ImGui::Text("CPU Usage");
    ImGui::PlotLines("##CPU", cpuHistory, HISTORY_SIZE,
        0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));

    ImGui::Spacing();

    ImGui::Text("Memory Usage");
    ImGui::PlotLines("##Memory", memoryHistory, HISTORY_SIZE,
        0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));

    ImGui::Spacing();
    ImGui::Text("CPU: %.1f%%    Memory: %.1f%%",
        cpuHistory[HISTORY_SIZE - 1],
        memoryHistory[HISTORY_SIZE - 1]);
}

// shift old data left, append new dummy data point
void TaskManager::updatePerformanceData()
{
    for (int i = 0; i < HISTORY_SIZE - 1; i++)
    {
        cpuHistory[i] = cpuHistory[i + 1];
        memoryHistory[i] = memoryHistory[i + 1];
    }

    float baseCpu = 14.0f;
    float baseMem = 42.0f;
    cpuHistory[HISTORY_SIZE - 1] = baseCpu + (float)(rand() % 20) - 10.0f;
    memoryHistory[HISTORY_SIZE - 1] = baseMem + (float)(rand() % 10) - 5.0f;

    // clamp
    if (cpuHistory[HISTORY_SIZE - 1] < 0.0f) cpuHistory[HISTORY_SIZE - 1] = 0.0f;
    if (memoryHistory[HISTORY_SIZE - 1] < 0.0f) memoryHistory[HISTORY_SIZE - 1] = 0.0f;
}

ImVec4 TaskManager::getStateColor(ProcessState state)
{
    switch (state)
    {
        case ProcessState::RUNNING:
            return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);   // green
        case ProcessState::READY:
            return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);   // yellow
        case ProcessState::WAITING:
            return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);   // orange
        case ProcessState::TERMINATED:
            return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);   // gray
        default:
            return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // white
    }
}

const char* TaskManager::getStateName(ProcessState state)
{
    switch (state)
    {
        case ProcessState::RUNNING:    return "Running";
        case ProcessState::READY:      return "Ready";
        case ProcessState::WAITING:    return "Waiting";
        case ProcessState::TERMINATED: return "Terminated";
        default:                       return "Unknown";
    }
}