#pragma once



#include <stdint.h>



struct GLFWwindow;



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
	GLFWwindow* m_pWindow = nullptr;
	uint32_t m_screenWidth = 1366;
	uint32_t m_screenHeight = 768;
	bool m_IsInitialised = false;
	bool m_showImguiDemoWindow = false;
	bool m_showImplotDemoWindow = false;

	static void GLFWErrorCallback(int error, const char* description);
	static void OnWindowResize(GLFWwindow* pWindow, int width, int height);
};