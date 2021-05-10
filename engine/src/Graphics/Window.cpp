#include "Window.h"
#include <Core/Logger.h>

namespace vke
{
	VkeWindow::VkeWindow(int w, int h, std::string name) : m_Width(w), m_Height(h), m_WindowName(name), m_Window(nullptr)
	{
		InitWindow();
	}

	VkeWindow::~VkeWindow()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void VkeWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create window surface");
		}
	}

	void VkeWindow::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
	}
}