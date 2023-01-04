#include "App.h"
#include "MessageBus.h"
#include <memory>



int main(int argc, char* argv[])
{
    // create message bus
    std::shared_ptr<MessageBus> pMessageBus = std::make_shared<MessageBus>();

	// create app
    std::shared_ptr<App> pApp = std::make_shared<App>(pMessageBus);
    pMessageBus->AddReceiver(pApp);

    // run app
    pApp->Run();

    // shutdown
    pApp.reset();
    pMessageBus.reset();
    return 0;
}