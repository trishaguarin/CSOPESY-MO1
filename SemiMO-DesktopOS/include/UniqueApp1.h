#pragma once
#include "AppWindow.h"
#include <string>

class UniqueApp1 : public AppWindow {
public:
    UniqueApp1();
    void draw() override;

private:
    std::string display = "0";
    std::string firstOperand = "";
    char pendingOp = '\0';
    bool freshInput = true;

    void pressDigit(const char* d);
    void pressOp(char op);
    void pressEquals();
    void pressClear();
    void pressDecimal();
    void pressBackspace();
};