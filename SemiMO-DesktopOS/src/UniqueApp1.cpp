//calculator nalang natin toh guys
// placeholder — to be implemented later

#include "UniqueApp1.h"
#include "imgui.h"

UniqueApp1::UniqueApp1()
    : AppWindow("Calculator")
{
}

void UniqueApp1::draw()
{
    if (!beginWindow())
        return;

    ImGui::Text("Calculator — Coming Soon");

    endWindow();
}