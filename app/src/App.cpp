#include "App.h"
#include <Core/Logger.h>
#include "Graphics/SimpleRenderSystem.h"

#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace vke
{
	App::App() : m_Window(WIDTH, HEIGHT, "Hello Vulkan!"), m_Device(m_Window), m_Renderer(m_Window, m_Device)
	{
		LoadGameObjects();
	}

	App::~App()
	{
	}

	void App::Run()
	{
		SimpleRenderSystem simpleRenderSystem{ m_Device, m_Renderer.GetSwapChainRenderPass() };

		while (!m_Window.ShouldClose())
		{
			glfwPollEvents();

			if (auto commandBuffer = m_Renderer.BeginFrame())
			{
				m_Renderer.BeginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.RenderGameObjects(commandBuffer, m_GameObjects);
				m_Renderer.EndSwapChainRenderPass(commandBuffer);
				m_Renderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(m_Device.GetDevice());
	}

	void App::LoadGameObjects()
	{
		std::vector<Model::Vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}} };
		auto lveModel = std::make_shared<Model>(m_Device, vertices);

		auto triangle = GameObject::CreateGameObject();
		triangle.model = lveModel;
		triangle.color = { .1f, .8f, .1f };
		triangle.transform.translation.x = .2f;
		triangle.transform.scale = { 2.f, .5f };
		triangle.transform.rotation = .25f * glm::two_pi<float>();

		m_GameObjects.push_back(std::move(triangle));
	}
}