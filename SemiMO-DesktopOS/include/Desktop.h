#pragma once

#include "AppWindow.h"

class Desktop : public AppWindow
{
public:
    Desktop();

    void draw() override;

    void setAppRunning(bool* running);

private:
    void renderWallpaper();
    void drawClock();
    void powerButton();

    bool* appRunning = nullptr;
};