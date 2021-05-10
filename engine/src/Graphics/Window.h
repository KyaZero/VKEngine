#pragma once
#include <Core/Types.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vke
{
	class VkeWindow
	{
	public:
		VkeWindow(int w, int h, std::string name);
		~VkeWindow();

		VkeWindow(const VkeWindow&) = delete;
		VkeWindow& operator=(const VkeWindow&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(m_Window); }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height) }; }

	private:
		void InitWindow();

		const i32 m_Width;
		const i32 m_Height;
		std::string m_WindowName;
		GLFWwindow* m_Window;
	};
}