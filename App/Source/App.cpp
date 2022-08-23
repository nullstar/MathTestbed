#include "App.h"
#include "Widgets/SecondOrderDynamicsWidget.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <implot.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <stdio.h>
#include <typeinfo>
#include <chrono>



App::App()
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
    Widgets.clear();

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
        if (m_showImguiDemoWindow)
            ImGui::ShowDemoWindow(&m_showImguiDemoWindow);

        if (m_showImplotDemoWindow)
            ImPlot::ShowDemoWindow(&m_showImplotDemoWindow);

        // udpate main menu
        MainMenu();

        // render widgets
        for(size_t widgetIndex = 0; widgetIndex < Widgets.size(); ++widgetIndex)
        {
            const std::unique_ptr<IWindowWidget>& pWidget = Widgets[widgetIndex];
            if (pWidget->WantsToClose())
            {
                std::swap(Widgets[widgetIndex], Widgets.back());
                Widgets.pop_back();
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
		if (ImGui::BeginMenu("Interpolation"))
		{
            if (ImGui::MenuItem("Second Order Dynamics"))
                CreateUniqueWidget<SecondOrderDynamicsWidget>();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Demos"))
		{
			if (ImGui::MenuItem("Imgui"))
				m_showImguiDemoWindow = !m_showImguiDemoWindow;

			if (ImGui::MenuItem("Implot"))
				m_showImplotDemoWindow = !m_showImplotDemoWindow;

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}


template<typename widgetType>
void App::CreateUniqueWidget()
{
    // if we already have a widget of this type then give it focus and return
    const std::type_info& widgetTypeInfo = typeid(widgetType);
    for (const std::unique_ptr<IWindowWidget>& pWidget : Widgets)
    {
        if (typeid(*pWidget) == widgetTypeInfo)
        {
            pWidget->Focus();
            return;
        }
    }

    // otherwise create a new widget of the given type
    Widgets.push_back(std::make_unique<widgetType>());
}