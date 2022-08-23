#include "App.h"
#include <memory>



int main(int argc, char* argv[])
{
	// create app
    std::unique_ptr<App> pApp = std::make_unique<App>();
    if (!pApp)
        return -1;

    // run app
    pApp->Run();

    // shutdown
    pApp.reset();
    return 0;
}