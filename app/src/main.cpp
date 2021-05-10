#include <iostream>
#include <Core/Logger.h>
#include "App.h"

int main()
{
	vke::Logger::Create(false);
	vke::Logger::SetLevel(vke::Logger::Level::Info);

	{
		vke::App app{};

		app.Run();
	}

	vke::Logger::Destroy();
	return EXIT_SUCCESS;
}