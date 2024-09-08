#pragma once

#include "Zahra/Core/Base.h"
#include "Zahra/Core/Application.h"

#ifdef Z_PLATFORM_WINDOWS

extern Zahra::Application* Zahra::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	Zahra::Log::Init();
	
	Z_PROFILE_BEGIN_SESSION("Startup", "C:/dev/Zahra/profiling/Zahra_profile_startup.json");
	auto app = Zahra::CreateApplication({ argc, argv });
	Z_PROFILE_END_SESSION();

	Z_PROFILE_BEGIN_SESSION("Runtime", "C:/dev/Zahra/profiling/Zahra_profile_runtime.json");
	try {
		app->Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	Z_PROFILE_END_SESSION();

	Z_PROFILE_BEGIN_SESSION("Shutdown", "C:/dev/Zahra/profiling/Zahra_profile_shutdown.json");
	delete app;
	Z_PROFILE_END_SESSION();
}

#endif
