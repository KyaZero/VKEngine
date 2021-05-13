#pragma once
#include "Graphics/Window.h"
#include "Graphics/Device.h"
#include "Graphics/SwapChain.h"
#include "Graphics/Model.h"
#include <Core/Logger.h>

#include <memory>
#include <vector>

namespace vke
{
	class Renderer
	{
	public:
		Renderer(Window& window, Device& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		VkRenderPass GetSwapChainRenderPass() const { return m_SwapChain->GetRenderPass(); }
		bool IsFrameInProgress() const { return m_IsFrameStarted; }

		VkCommandBuffer GetCurrentCommandBuffer() const 
		{ 
			ASSERT_LOG(m_IsFrameStarted, "Cannot get command buffer when frame not in progress!");
			return m_CommandBuffers[m_CurrentFrameIndex];
		}

		int GetFrameIndex() const 
		{
			ASSERT_LOG(m_IsFrameStarted, "Cannot get frame index when frame not in progress!");
			return m_CurrentFrameIndex;
		}

		VkCommandBuffer BeginFrame();
		void EndFrame();

		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void RecreateSwapChain();

		Window& m_Window;
		Device& m_Device;
		std::unique_ptr<SwapChain> m_SwapChain;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		uint32_t m_CurrentImageIndex;
		int m_CurrentFrameIndex;
		bool m_IsFrameStarted;
	};
}