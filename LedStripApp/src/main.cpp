#include <iostream>
#include "app.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    App app = App();
    if (!app.Init())
    {
        std::cout << "Failed to create the App" << std::endl;
        return EXIT_FAILURE;
    }

    app.Run();
    return EXIT_SUCCESS;
}
