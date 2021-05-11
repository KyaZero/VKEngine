#include "App.h"
#include <Core/Logger.h>

#include <array>

namespace vke
{
	App::App() : m_Window(WIDTH, HEIGHT, "Hello Vulkan!"), m_Device(m_Window), m_SwapChain(m_Device, m_Window.GetExtent()), m_Pipeline(), m_PipelineLayout()
	{
		LoadModels();
		CreatePipelineLayout();
		CreatePipeline();
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
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_Device.Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create Pipeline Layout!");
		}
	}

	void App::CreatePipeline()
	{
		PipelineConfigInfo pipelineConfig{};
		VkePipeline::DefaultPipelineConfigInfo(pipelineConfig, m_SwapChain.Width(), m_SwapChain.Height());
		pipelineConfig.renderPass = m_SwapChain.GetRenderPass();
		pipelineConfig.pipelineLayout = m_PipelineLayout;

		m_Pipeline = std::make_unique<VkePipeline>(m_Device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
	}

	void App::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(m_SwapChain.ImageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_Device.GetCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

		if (vkAllocateCommandBuffers(m_Device.Device(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to allocate Command Buffers!")
		}

		for (int i = 0; i < m_CommandBuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				FATAL_LOG("Failed to begin recording Command Buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_SwapChain.GetRenderPass();
			renderPassInfo.framebuffer = m_SwapChain.GetFrameBuffer(i);

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_SwapChain.GetSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 1 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			m_Pipeline->Bind(m_CommandBuffers[i]);
			m_Model->Bind(m_CommandBuffers[i]);
			m_Model->Draw(m_CommandBuffers[i]);

			vkCmdEndRenderPass(m_CommandBuffers[i]);
			if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS)
			{
				FATAL_LOG("Failed to record Command Buffer!");
			}
		}
	}

	void App::DrawFrame()
	{
		uint32_t imageIndex;
		VkResult result = m_SwapChain.AcquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			FATAL_LOG("Failed to acquire swap chain image!");
		}

		result = m_SwapChain.SubmitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS)
		{
			FATAL_LOG("Failed to present swap chain image!");
		}
	}
}