#include <Zahra.h>
#include <Zahra/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Zahra
{
	class Meadow : public Application
	{
	public:

		Meadow(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new EditorLayer());
		}

		~Meadow()
		{

		}


	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Meadow";
		spec.WorkingDirectory = ".";
		spec.CommandLineArgs = args;

		return new Meadow(spec);
	}

}

