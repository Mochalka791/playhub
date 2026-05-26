#include "core/Application.h"
#include <iostream>

int main(int argc, char* argv[])
{
    Application app;
    if (!app.init("Casino", 1024, 768)) {
        std::cerr << "Failed to initialize application.\n";
        return 1;
    }
    app.run();
    return 0;
}
