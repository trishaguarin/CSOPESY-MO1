#pragma once

#include "AppWindow.h"
#include <string>
#include <glad/glad.h>

class Desktop : public AppWindow
{
public:
    Desktop();

    void draw() override;

    void setAppRunning(bool* running);

    // wallpaper
    void loadWallpaper(const char* path);

private:
    void renderWallpaper();
    void drawClock();
    void powerButton();

    bool* appRunning = nullptr;
    GLuint wallpaperTex = 0;
    std::string wallpaperPath;
};