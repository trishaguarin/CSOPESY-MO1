// UniqueApp1.cpp — Calculator
#include "UniqueApp1.h"
#include "imgui.h"
#include <string>
#include <cstdlib>
#include <cmath>
#include <cstdio>

UniqueApp1::UniqueApp1() : AppWindow("Calculator") {}


void UniqueApp1::pressDigit(const char* d) {
    if (freshInput) { display = d; freshInput = false; }
    else if (display == "0") display = d;
    else display += d;
}

void UniqueApp1::pressDecimal() {
    if (freshInput) { display = "0."; freshInput = false; return; }
    for (char c : display) if (c == '.') return;
    display += '.';
}

void UniqueApp1::pressBackspace() {
    if (freshInput || display.size() <= 1) { display = "0"; freshInput = false; return; }
    display.pop_back();
    if (display.empty() || display == "-") display = "0";
}

void UniqueApp1::pressOp(char op) {
    firstOperand = display;
    pendingOp    = op;
    freshInput   = true;
}

void UniqueApp1::pressEquals() {
    if (pendingOp == '\0' || firstOperand.empty()) return;

    double a = atof(firstOperand.c_str());
    double b = atof(display.c_str());
    double result = 0.0;

    switch (pendingOp) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/': result = (b != 0.0) ? a / b : 0.0; break;
    }

    // format: show as integer if no fractional part
    char buf[64];
    if (result == (long long)result)
        snprintf(buf, sizeof(buf), "%lld", (long long)result);
    else
        snprintf(buf, sizeof(buf), "%.8g", result);

    display      = buf;
    firstOperand = "";
    pendingOp    = '\0';
    freshInput   = true;
}

void UniqueApp1::pressClear() {
    display      = "0";
    firstOperand = "";
    pendingOp    = '\0';
    freshInput   = true;
}

// draw 

void UniqueApp1::draw() {
    if (!beginWindow()) return;

    ImGui::SetNextWindowSize(ImVec2(280, 380), ImGuiCond_Once);

    // display screen 
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.08f, 1.0f));
    ImGui::BeginChild("##screen", ImVec2(-1, 60), true);

    // show pending operation hint
    if (pendingOp != '\0') {
        char hint[64];
        snprintf(hint, sizeof(hint), "%s %c", firstOperand.c_str(), pendingOp);
        ImGui::TextDisabled("%s", hint);
    } else {
        ImGui::TextDisabled(" ");
    }

    // main number display, right-aligned
    ImVec2 textSz = ImGui::CalcTextSize(display.c_str());
    float  padX   = ImGui::GetContentRegionAvail().x - textSz.x;
    if (padX > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padX);
    ImGui::SetWindowFontScale(1.4f);
    ImGui::TextColored(ImVec4(1,1,1,1), "%s", display.c_str());
    ImGui::SetWindowFontScale(1.0f);

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // button grid
    float btnW = 58.0f;
    float btnH = 42.0f;

    // color helpers
    auto pushNum = [&]() {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.38f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.40f, 0.50f, 1.0f));
    };
    auto pushOp = [&]() {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.38f, 0.65f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.52f, 0.85f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.15f, 0.28f, 0.50f, 1.0f));
    };
    auto pushRed = [&]() {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.08f, 0.08f, 1.0f));
    };
    auto pop3 = [&]() { ImGui::PopStyleColor(3); };

    // row 1: C  <-  %  /
    pushRed(); if (ImGui::Button("C",  ImVec2(btnW, btnH))) pressClear();    pop3();
    ImGui::SameLine();
    pushNum(); if (ImGui::Button("<-", ImVec2(btnW, btnH))) pressBackspace(); pop3();
    ImGui::SameLine();
    pushNum(); if (ImGui::Button("%",  ImVec2(btnW, btnH))) {
        double v = atof(display.c_str()) / 100.0;
        char buf[64]; snprintf(buf, sizeof(buf), "%.8g", v);
        display = buf; freshInput = true;
    } pop3();
    ImGui::SameLine();
    pushOp(); if (ImGui::Button("/",   ImVec2(btnW, btnH))) pressOp('/');    pop3();

    // row 2: 7  8  9  x
    pushNum(); if (ImGui::Button("7",  ImVec2(btnW, btnH))) pressDigit("7"); pop3(); ImGui::SameLine();
    pushNum(); if (ImGui::Button("8",  ImVec2(btnW, btnH))) pressDigit("8"); pop3(); ImGui::SameLine();
    pushNum(); if (ImGui::Button("9",  ImVec2(btnW, btnH))) pressDigit("9"); pop3(); ImGui::SameLine();
    pushOp(); if (ImGui::Button("x",   ImVec2(btnW, btnH))) pressOp('*');    pop3();

    // row 3: 4  5  6  -
    pushNum(); if (ImGui::Button("4",  ImVec2(btnW, btnH))) pressDigit("4"); pop3(); ImGui::SameLine();
    pushNum(); if (ImGui::Button("5",  ImVec2(btnW, btnH))) pressDigit("5"); pop3(); ImGui::SameLine();
    pushNum(); if (ImGui::Button("6",  ImVec2(btnW, btnH))) pressDigit("6"); pop3(); ImGui::SameLine();
    pushOp(); if (ImGui::Button("-",   ImVec2(btnW, btnH))) pressOp('-');    pop3();

    // row 4: 1  2  3  +
    pushNum(); if (ImGui::Button("1",  ImVec2(btnW, btnH))) pressDigit("1"); pop3(); ImGui::SameLine();
    pushNum(); if (ImGui::Button("2",  ImVec2(btnW, btnH))) pressDigit("2"); pop3(); ImGui::SameLine();
    pushNum(); if (ImGui::Button("3",  ImVec2(btnW, btnH))) pressDigit("3"); pop3(); ImGui::SameLine();
    pushOp(); if (ImGui::Button("+",   ImVec2(btnW, btnH))) pressOp('+');    pop3();

    // row 5: +/-  0  .  =
    pushNum(); if (ImGui::Button("+/-", ImVec2(btnW, btnH))) {
        if (!display.empty() && display != "0") {
            if (display[0] == '-') display.erase(0, 1);
            else display = "-" + display;
        }
    } pop3();
    ImGui::SameLine();
    pushNum(); if (ImGui::Button("0",  ImVec2(btnW, btnH))) pressDigit("0"); pop3(); ImGui::SameLine();
    pushNum(); if (ImGui::Button(".",  ImVec2(btnW, btnH))) pressDecimal();  pop3(); ImGui::SameLine();
    pushOp(); if (ImGui::Button("=",   ImVec2(btnW, btnH))) pressEquals();   pop3();

    endWindow();
}
