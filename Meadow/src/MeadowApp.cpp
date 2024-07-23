#include <Zahra.h>
#include <Zahra/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Zahra
{
	class Meadow : public Application
	{
	public:

		Meadow(ApplicationCommandLineArgs args)
			: Application("Meadow", args)
		{
			//PushLayer(new ExampleLayer());
			PushLayer(new EditorLayer());
		}

		~Meadow()
		{

		}


	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new Meadow(args);
	}

}

