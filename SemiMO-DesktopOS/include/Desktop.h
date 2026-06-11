#pragma once

class Desktop
{
public:
    void draw(bool* appRunning);

private:
    void renderWallpaper();
    void drawClock();
    void powerButton(bool* appRunning);
};