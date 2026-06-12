#pragma once
#include <string>
#include "imgui.h"

class AppWindow {
    public:
        AppWindow(const std::string& name);
        virtual ~AppWindow() = default;
        virtual void draw() = 0;

        void show();
        void hide();
        bool isShown() const;

    protected:
        bool beginWindow();
        void endWindow();

        std::string windowName;
        bool isVisible;
};