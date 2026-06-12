#pragma once
#include "TaskManager.h"
#include "UniqueApp1.h"
#include "UniqueApp2.h"

class Taskbar
{
public:
    void draw(TaskManager& taskMgr, UniqueApp1& app1, UniqueApp2& app2);

private:
    void drawSystemTray();
};