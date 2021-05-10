#pragma once
#include "Graphics/Window.h"
#include "Graphics/Device.h"
#include "Graphics/SwapChain.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Model.h"

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
		void LoadModels();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffers();
		void DrawFrame();

		VkeWindow m_Window;
		VkeDevice m_Device;
		VkeSwapChain m_SwapChain;
		std::unique_ptr<VkePipeline> m_Pipeline;
		VkPipelineLayout m_PipelineLayout;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::unique_ptr<VkeModel> m_Model;
	};
}