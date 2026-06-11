#include "GUIApplication.h"

int main()
{
    GUIApplication app;

    if (!app.Initialize())
        return -1;

    app.Run();
    app.Shutdown();

    return 0;
}