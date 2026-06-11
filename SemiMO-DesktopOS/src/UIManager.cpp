#include "UIManager.h"
#include "imgui.h"

void UIManager::Render()
{
    switch (currentState)
    {
        case OSState::Boot:
            RenderBootScreen();
            break;
        case OSState::Setup:
            RenderSetupScreen();
            break;
        case OSState::Desktop:
            RenderDesktop();
            break;
    }
}

void UIManager::RenderBootScreen()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("Boot", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_NoMove
    );

    ImGui::Text("CSOPESY Megatrends");
    ImGui::Text("Released: %s", currentTime.c_str());
    ImGui::Separator();

    ImGui::Text("Memory Test:");
    ImGui::Text("Checking RAM: 8192 MB");
    ImGui::Spacing();

    ImGui::Text("CPU Type:          Intel Core i7-12700K");
    ImGui::Text("BIOS Version:      SemiMO v1.0.0");
    ImGui::Text("Main Processor:    3.60 GHz");
    ImGui::Text("Numeric Processor: Installed");
    ImGui::Spacing();

    ImGui::Text("Primary Master:    SemiMO SSD 512GB");
    ImGui::Text("Primary Slave:     None");
    ImGui::Text("Secondary Master:  None");
    ImGui::Text("Secondary Slave:   None");
    ImGui::Spacing();

    ImGui::Text("Fun Fact: The first computer bug was an actual bug — a moth found in a relay.");
    ImGui::Spacing();

    ImGui::Text("Press [DEL] to enter setup");

    ImGui::End();
}

void UIManager::RenderSetupScreen()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("Setup", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_NoMove
    );

    ImGui::Text("SemiMO Setup Screen");
    ImGui::Separator();
    ImGui::Text("Loading...");

    // Placeholder loading bar
    static float progress = 0.0f;
    progress += 0.001f;
    if (progress > 1.0f) progress = 0.0f;
    ImGui::ProgressBar(progress, ImVec2(-1, 0), "Loading...");

    ImGui::End();
}

void UIManager::RenderDesktop()
{
    // Desktop rendering is handled by Desktop class
    // This is a placeholder
}