#include "App.h"
#include "Widgets/ExponentialDecayWidget.h"
#include "Widgets/EncyclopediaWidget.h"
#include "Widgets/SecondOrderDynamicsWidget.h"
#include <chrono>
#include <imgui.h>
#include <implot.h>
#include <sstream>
#include <stdio.h>
#include <typeinfo>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>



#define SHOW_IMGUI_DEMOS 0



App::App(std::weak_ptr<MessageBus> pMessageBus)
    : m_pMessageBus(pMessageBus)
{
	// initialise glfw
    glfwSetErrorCallback(App::GLFWErrorCallback);
    if (!glfwInit())
        return;

    // set GLSL version
    const char* GLSLVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // request multi-sampling
    glfwWindowHint(GLFW_SAMPLES, 4);

    // create window
    m_pWindow = glfwCreateWindow(m_screenSize.x, m_screenSize.y, GetWindowName(), NULL, NULL);
    if (!m_pWindow)
    {
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_pWindow);
    
    glfwSetWindowUserPointer(m_pWindow, this);                      // set user pointer
    glfwSetWindowSizeCallback(m_pWindow, App::OnWindowResize);      // bind window resize callback
    glfwSwapInterval(1);                                            // enable v-sync

    // set successful initialisation
    m_IsInitialised = true;

    // setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // setup ImPlot context
    ImPlot::CreateContext(); 

    // setup platform / renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    ImGui_ImplOpenGL3_Init(GLSLVersion);
}


App::~App()
{
    // release widgets
    m_widgets.clear();

    // shutdown imgui
    if (m_IsInitialised)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    // destroy the window
    if (m_pWindow)
    {
        glfwDestroyWindow(m_pWindow);
        m_pWindow = nullptr;
    }

    // clean up glfw
    glfwTerminate();
}


const char* App::GetWindowName() const
{
    return "Testbed";
}


void App::Run()
{
    if (!m_IsInitialised)
        return;

    // execute run loop
    while (!glfwWindowShouldClose(m_pWindow))
    {
        // update message bus
        if (std::shared_ptr<MessageBus> pMessageBus = m_pMessageBus.lock())
            pMessageBus->BroadcastMessages();

        // poll for and process events
        glfwPollEvents();

        // start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // clear frame buffer and setup rendering
        int frameBufferWidth, frameBufferHeight;
        glfwGetFramebufferSize(m_pWindow, &frameBufferWidth, &frameBufferHeight);
        glViewport(0, 0, frameBufferWidth, frameBufferHeight);
        glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // show the demo windows
#if SHOW_IMGUI_DEMOS
        if (m_showImguiDemoWindow)
            ImGui::ShowDemoWindow(&m_showImguiDemoWindow);

        if (m_showImplotDemoWindow)
            ImPlot::ShowDemoWindow(&m_showImplotDemoWindow);
#endif // SHOW_IMGUI_DEMOS

        // udpate main menu
        MainMenu();

        // render widgets
        for(size_t widgetIndex = 0; widgetIndex < m_widgets.size(); ++widgetIndex)
        {
            const std::shared_ptr<IWindowWidget>& pWidget = m_widgets[widgetIndex];
            if (pWidget->WantsToClose())
            {
                std::swap(m_widgets[widgetIndex], m_widgets.back());
                m_widgets.pop_back();
                --widgetIndex;
            }
            else
            {
                pWidget->Render();
            }
        }

        // render imgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // swap frame buffers
        glfwSwapBuffers(m_pWindow);
    }
}


void App::SendMessage(const MessageType& message) const
{
    if (std::shared_ptr<MessageBus> pMessageBus = m_pMessageBus.lock())
        pMessageBus->AddMessage(message);
}


void App::OnMessage(const MessageType& message)
{
    // determine message type from first token
    std::istringstream stream(message);
    std::string messageType;
    stream >> messageType;

    if (messageType == "OpenWindow")
    {
        // determine window to open
        std::string windowType;
        stream >> windowType;

        if (windowType == "Encyclopedia")
        {
            CreateUniqueWidget<EncyclopediaWidget>();

            // send message to turn encyclopedia to correct page
            std::string encyclopediaPage;
            stream >> encyclopediaPage;
            if (!encyclopediaPage.empty())
            {
                SendMessage("GotoEncyclopediaPage " + encyclopediaPage);
            }
        }
        else if (windowType == "ExponentialDecay")
        {
            CreateUniqueWidget<ExponentialDecayWidget>();
        }
        else if (windowType == "SecondOrderDynamics")
        {
            CreateUniqueWidget<SecondOrderDynamicsWidget>();
        }
    }
}


void App::GLFWErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


void App::OnWindowResize(GLFWwindow* pWindow, int width, int height)
{
    App* pApp = static_cast<App*>(glfwGetWindowUserPointer(pWindow));
    if (pApp)
    {
        pApp->m_screenSize.x = width;
        pApp->m_screenSize.y = height;
    }
}


void App::MainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
        if (ImGui::MenuItem("Encyclopedia"))
            SendMessage("OpenWindow Encyclopedia");

        if (ImGui::BeginMenu("Demos"))
        {
            if (ImGui::BeginMenu("Interpolation"))
            {
                if (ImGui::MenuItem("Second Order Dynamics"))
                    SendMessage("OpenWindow SecondOrderDynamics");

                if (ImGui::MenuItem("Exponential Decay"))
                    SendMessage("OpenWindow ExponentialDecay");

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

#if SHOW_IMGUI_DEMOS
		if (ImGui::BeginMenu("Imgui Demos"))
		{
			if (ImGui::MenuItem("Imgui"))
				m_showImguiDemoWindow = !m_showImguiDemoWindow;

			if (ImGui::MenuItem("Implot"))
				m_showImplotDemoWindow = !m_showImplotDemoWindow;

			ImGui::EndMenu();
		}
#endif // #if SHOW_IMGUI_DEMOS

		ImGui::EndMainMenuBar();
	}
}


template<typename widgetType>
void App::CreateUniqueWidget()
{
    // if we already have a widget of this type then give it focus and return
    const std::type_info& widgetTypeInfo = typeid(widgetType);
    for (const std::shared_ptr<IWindowWidget>& pWidget : m_widgets)
    {
        if (typeid(*pWidget) == widgetTypeInfo)
        {
            pWidget->Focus();
            return;
        }
    }

    // otherwise create a new widget of the given type and add it to the message bus
    std::shared_ptr<widgetType> pWidget = std::make_shared<widgetType>(m_pMessageBus);
    m_widgets.push_back(pWidget);

    if (std::shared_ptr<MessageBus> pMessageBus = m_pMessageBus.lock())
        pMessageBus->AddReceiver(pWidget);
}