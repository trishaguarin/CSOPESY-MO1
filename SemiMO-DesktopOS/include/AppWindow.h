#pragma once
#include <string>
#include "imgui.h"

class AppWindow {
    public:
        AppWindow(const std::string& name) : windowName(name), isVisible(false) {}
        virtual ~AppWindow() = default;
        virtual void draw() = 0;

        void show() { isVisible = true; }
        void hide() { isVisible = false; }
        bool isShown() const { return isVisible; }

    protected:
        bool beginWindow() {
            if (!isVisible) return false;
            ImGui::Begin(windowName.c_str(), &isVisible);
            return true;
        }
        void endWindow() { ImGui::End(); }

        std::string windowName;
        bool isVisible;
};