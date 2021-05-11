#pragma once
#include "Graphics/Window.h"
#include "Graphics/Device.h"
#include "Graphics/SwapChain.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Model.h"
#include "GameObject.h"

#include <memory>
#include <vector>

namespace vke
{
	class App
	{
	public:
		App();
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void Run();

		static constexpr i32 WIDTH = 800;
		static constexpr i32 HEIGHT = 600;

	private:
		void LoadGameObjects();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void DrawFrame();
		void RecreateSwapChain();
		void RecordCommandBuffer(int imageIndex);
		void RenderGameObjects(VkCommandBuffer commandBuffer);

		Window m_Window;
		GraphicsDevice m_Device;
		std::unique_ptr<GraphicsSwapChain> m_SwapChain;
		std::unique_ptr<GraphicsPipeline> m_Pipeline;
		VkPipelineLayout m_PipelineLayout;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::vector<GameObject> m_GameObjects;
	};
}