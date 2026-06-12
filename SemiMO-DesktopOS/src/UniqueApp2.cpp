#include "UniqueApp2.h"
#include "imgui.h"
#include <cstring>
#include <ctime>

UniqueApp2::UniqueApp2() : AppWindow("Notepad") {
    const char* placeholder =
        "Start typing your notes here...\n";
    strncpy(noteBuffer, placeholder, sizeof(noteBuffer) - 1);
}

void UniqueApp2::draw() {
    ImGui::SetNextWindowSize(ImVec2(520, 440), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(420, 320), ImVec2(800, 600));

    if (!beginWindow()) return;

    // top toolbar
    if (ImGui::Button("Clear")) {
        noteBuffer[0] = '\0';
        unsavedChanges = false;
        ImGui::SetKeyboardFocusHere(-1);
    }

    // word count
    int wordCount = 0;
    bool inWord = false;
    for (int i = 0; noteBuffer[i] != '\0'; i++) {
        if (noteBuffer[i] != ' ' && noteBuffer[i] != '\n' && noteBuffer[i] != '\t') {
            if (!inWord) { wordCount++; inWord = true; }
        } else {
            inWord = false;
        }
    }
    int charCount = (int)strlen(noteBuffer);
    ImGui::TextDisabled("chars: %d  words: %d", charCount, wordCount);

    if (unsavedChanges) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.1f, 1.0f), "● unsaved");
    }

    ImGui::Separator();

    ImVec2 contentAvail = ImGui::GetContentRegionAvail();
    ImVec2 textAreaSize = ImVec2(contentAvail.x, contentAvail.y);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,    ImVec4(0.95f, 0.95f, 0.95f, 1.0f));

    if (ImGui::InputTextMultiline(
        "##notes",
        noteBuffer,
        sizeof(noteBuffer),
        textAreaSize,
        ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_NoHorizontalScroll))
    {
        unsavedChanges = true;
    }

    ImGui::PopStyleColor(2);

    endWindow();
}