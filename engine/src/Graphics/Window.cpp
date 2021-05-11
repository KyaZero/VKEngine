#include "Window.h"
#include <Core/Logger.h>

namespace vke
{
	Window::Window(int w, int h, std::string name) : m_Width(w), m_Height(h), m_WindowName(name), m_Window(nullptr), m_FramebufferResized(false)
	{
		InitWindow();
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create window surface");
		}
	}

	void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto* vkeWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		vkeWindow->m_FramebufferResized = true;
		vkeWindow->m_Width = width;
		vkeWindow->m_Height = height;
	}

	void Window::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
	}
}