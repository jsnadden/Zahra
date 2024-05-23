#pragma once

#include "Core.h"

namespace Zahra
{
	class ZAHRA_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined by client app
	Application* CreateApplication();
}
