#pragma once
#include "Graphics/Window.h"
#include "Graphics/Device.h"
#include "Graphics/Pipeline.h"

namespace vke
{
	class App
	{
	public:
		App();
		~App();

		void Run();

		static constexpr i32 WIDTH = 800;
		static constexpr i32 HEIGHT = 600;

	private:
		VkeWindow m_Window;
		VkeDevice m_Device;
		VkePipeline m_Pipeline;
	};
}