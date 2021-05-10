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
		VkeWindow(i32 w, i32 h, std::string name);
		~VkeWindow();

		VkeWindow(const VkeWindow&) = delete;
		VkeWindow& operator=(const VkeWindow&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(m_Window); }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		void InitWindow();

		const i32 m_Width;
		const i32 m_Height;
		std::string m_WindowName;
		GLFWwindow* m_Window;
	};
}