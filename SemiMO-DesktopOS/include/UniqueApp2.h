#pragma once
#include "AppWindow.h"
#include <string>

class UniqueApp2 : public AppWindow {
public:
    UniqueApp2();
    void draw() override;

private:
    char noteBuffer[8192] = {};
    bool unsavedChanges = false;
};