#pragma once

#ifdef Z_PLATFORM_WINDOWS

extern Zahra::Application* Zahra::CreateApplication();

int main(int argc, char* argv[])
{
	Zahra::Log::Init();
	Z_CORE_WARN("Initialised core log");

	auto app = Zahra::CreateApplication();
	app->Run();
	delete app;
}

#endif