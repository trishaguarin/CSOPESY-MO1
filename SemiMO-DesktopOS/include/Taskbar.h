#pragma once
#include "TaskManager.h"
#include "UniqueApp1.h"
#include "UniqueApp2.h"

class Taskbar {
public:
    void draw();
    void setWindows(TaskManager* tm, UniqueApp1* a1, UniqueApp2* a2);

private:
    void drawSystemTray();

    TaskManager* taskMgr = nullptr;
    UniqueApp1*  app1    = nullptr;
    UniqueApp2*  app2    = nullptr;
    bool*        appRunning = nullptr;
};
