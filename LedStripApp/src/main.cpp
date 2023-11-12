#include <iostream>
#include "app.h"

// Main code
int main(int, char**)
{
    App* app = new App();
    if (!app->Init())
    {
        std::cout << "Failed to create an App" << std::endl;
        return -1;
    }

    app->Run();
    return 0;
}
