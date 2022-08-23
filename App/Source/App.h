#pragma once



#include <glm/glm.hpp>
#include <vector>
#include <memory>



struct GLFWwindow;
class IWindowWidget;



class App
{
public:
	App();
	~App();

	const char* GetWindowName() const;
	void Run();

	void ShowImguiDemoWindow() { m_showImguiDemoWindow = true; }
	void ShowImplotDemoWindow() { m_showImplotDemoWindow = true; }

private:
	static void GLFWErrorCallback(int error, const char* description);
	static void OnWindowResize(GLFWwindow* pWindow, int width, int height);
	void MainMenu();

	template<typename widgetType>
	void CreateUniqueWidget();

	GLFWwindow* m_pWindow = nullptr;
	glm::u32vec2 m_screenSize = glm::u32vec2(1366, 768);
	bool m_IsInitialised = false;

	bool m_showImguiDemoWindow = false;
	bool m_showImplotDemoWindow = false;

	std::vector<std::unique_ptr<IWindowWidget>> Widgets;
};