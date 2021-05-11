#include "App.h"
#include <Core/Logger.h>

#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vke
{
	struct PushConstantData
	{
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	App::App() : m_Window(WIDTH, HEIGHT, "Hello Vulkan!"), m_Device(m_Window), m_SwapChain(), m_Pipeline(), m_PipelineLayout()
	{
		LoadModels();
		CreatePipelineLayout();
		RecreateSwapChain();
		CreateCommandBuffers();
	}

	App::~App()
	{
		vkDestroyPipelineLayout(m_Device.Device(), m_PipelineLayout, nullptr);
	}

	void App::Run()
	{
		while (!m_Window.ShouldClose())
		{
			glfwPollEvents();
			DrawFrame();
		}

		vkDeviceWaitIdle(m_Device.Device());
	}

	void App::LoadModels()
	{
		std::vector<VkeModel::Vertex> vertices{
			{{  0.0f, -0.5f }, {1.0f, 0.0f, 0.0f}},
			{{  0.5f,  0.5f }, {0.0f, 1.0f, 0.0f}},
			{{ -0.5f,  0.5f }, {0.0f, 0.0f, 1.0f}}
		};

		m_Model = std::make_unique<VkeModel>(m_Device, vertices);
	}

	void App::CreatePipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device.Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create Pipeline Layout!");
		}
	}

	void App::CreatePipeline()
	{
		ASSERT_LOG(m_SwapChain != nullptr, "Cannot create pipeline before swap chain!");
		ASSERT_LOG(m_PipelineLayout != nullptr, "Cannot create pipeline before pipeline layout!");

		PipelineConfigInfo pipelineConfig {};
		VkePipeline::DefaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
		pipelineConfig.pipelineLayout = m_PipelineLayout;

		m_Pipeline = std::make_unique<VkePipeline>(m_Device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
	}

	void App::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(m_SwapChain->ImageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_Device.GetCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

		if (vkAllocateCommandBuffers(m_Device.Device(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to allocate Command Buffers!")
		}
	}

	void App::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(m_Device.Device(), m_Device.GetCommandPool(), static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
		m_CommandBuffers.clear();
	}

	void App::DrawFrame()
	{
		uint32_t imageIndex;
		VkResult result = m_SwapChain->AcquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			FATAL_LOG("Failed to acquire swap chain image!");
		}

		RecordCommandBuffer(imageIndex);
		result = m_SwapChain->SubmitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.WasWindowResized())
		{
			m_Window.ResetWindowResizedFlag();
			RecreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS)
		{
			FATAL_LOG("Failed to present swap chain image!");
		}
	}

	void App::RecreateSwapChain()
	{
		auto extent = m_Window.GetExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = m_Window.GetExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_Device.Device());

		if (m_SwapChain == nullptr)
		{
			m_SwapChain = std::make_unique<VkeSwapChain>(m_Device, extent);
		}
		else
		{
			m_SwapChain = std::make_unique<VkeSwapChain>(m_Device, extent, std::move(m_SwapChain));

			if (m_SwapChain->ImageCount() != m_CommandBuffers.size())
			{
				FreeCommandBuffers();
				CreateCommandBuffers();
			}
		}

		CreatePipeline();
	}

	void App::RecordCommandBuffer(int imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(m_CommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to begin recording Command Buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
		renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 1 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{ {0, 0}, m_SwapChain->GetSwapChainExtent() };
		vkCmdSetViewport(m_CommandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(m_CommandBuffers[imageIndex], 0, 1, &scissor);

		m_Pipeline->Bind(m_CommandBuffers[imageIndex]);
		m_Model->Bind(m_CommandBuffers[imageIndex]);

		for (int j = 0; j < 4; ++j)
		{
			PushConstantData push{};
			push.offset = { 0.0f, -0.4f + j * 0.25f };
			push.color = { 0.0f, 0.0f, 0.2f + 0.2f * j };

			vkCmdPushConstants(m_CommandBuffers[imageIndex], m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
			m_Model->Draw(m_CommandBuffers[imageIndex]);
		}

		vkCmdEndRenderPass(m_CommandBuffers[imageIndex]);
		if (vkEndCommandBuffer(m_CommandBuffers[imageIndex]) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to record Command Buffer!");
		}
	}
}