#include "App.h"

namespace vke
{
	App::App() : m_Window(WIDTH, HEIGHT, "Hello Vulkan!"), m_Device(m_Window), m_Pipeline(m_Device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", VkePipeline::DefaultPipelineConfigInfo(WIDTH, HEIGHT))
	{
	}

	App::~App()
	{
	}

	void App::Run()
	{
		while (!m_Window.ShouldClose())
		{
			glfwPollEvents();
		}
	}
}