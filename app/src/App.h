#pragma once
#include "Graphics/Window.h"
#include "Graphics/Device.h"
#include "Graphics/Renderer.h"
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

		Window m_Window;
		Device m_Device;
		Renderer m_Renderer;

		std::vector<GameObject> m_GameObjects;
	};
}