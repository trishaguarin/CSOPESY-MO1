
// medj iba ito sa pseudocode in the handout. pacheck nalang if this is usable. feel free to adjust. 
// but include as well the other funcs na andon sa handout kasi un ung register, show, hide, update. 

enum class OSState
{
    Boot,
    Setup,
    Desktop
};

void UIManager::Render() 
{
    switch(currentState)
    {
        case OSState::Boot:
            RenderBootScreen();
            break;

        case OSState::Setup:
            RenderSetupScreen(); //includes loading bar
            break;

        case OSState::Desktop:
            RenderDesktop();
            break;
    }
}

void UIManager::RenderBootScreen()
{
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin(
        "Boot",
        nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove
    );

    ImGui::Text("CSOPESY Megatrends");
    ImGui::Text("Released: %s", currentTime.c_str());

    ImGui::Separator();

    ImGui::Text("Memory Test:");
    ImGui::Text("Checking RAM: 8192 MB"); //check if keri kunin ung mismong RAM

    //put dummy values for each
    ImGui::Text("CPU Type: ");
    ImGui::Text("BIOS Version: ");
    ImGui::Text("Main Processor: ");
    ImGui::Text("Numeric Processor: ");

    ImGui::Text("Primary Master: ");
    ImGui::Text("Primary Slave: ");
    ImGui::Text("Secondary Master: ");
    ImGui::Text("Secondary Slave: ");

    ImGui::Spacing();

    ImGui::Text("Fun Fact: "); //add nlgn kayo funfact
    
    ImGui::Spacing();

    ImGui::Text("Press [DEL] to enter setup");

    ImGui::End();
}

//add states nalnag for the other OS States