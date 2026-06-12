//notes nalang toh guys
// placeholder — to be implemented later

#include "UniqueApp2.h"
#include "imgui.h"

UniqueApp2::UniqueApp2()
    : AppWindow("Notepad")
{
}

void UniqueApp2::draw()
{
    if (!beginWindow())
        return;

    ImGui::Text("Notepad — Coming Soon");

    endWindow();
}