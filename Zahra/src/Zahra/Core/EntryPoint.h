#pragma once

#include "Zahra/Core/Defines.h"
#include "Zahra/Core/Application.h"

#include <cstdlib>

#ifdef Z_PLATFORM_WINDOWS

extern Zahra::Application* Zahra::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	Zahra::Log::Init();
	
	auto app = Zahra::CreateApplication({ argc, argv });

	try {
		app->Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	delete app;

	return EXIT_SUCCESS;
}

#endif
