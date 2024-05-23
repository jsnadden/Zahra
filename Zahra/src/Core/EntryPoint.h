#pragma once

#ifdef Z_PLATFORM_WINDOWS

extern Zahra::Application* Zahra::CreateApplication();

int main(int argc, char* argv[])
{
	printf("Engine starting...\n");

	auto app = Zahra::CreateApplication();
	app->Run();
	delete app;
}

#endif