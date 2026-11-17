#include "app.h"
#include <cstring>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    CApp app;
    if (!app.init(hInstance, nCmdShow)) return 0;
    if (pika_contains(GetCommandLineA(), "--demo")) app.startDemo();
    return app.run();
}
