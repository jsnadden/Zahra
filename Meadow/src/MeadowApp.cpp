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
		spec.Version = ApplicationVersion(0, 1, 0);
		spec.WorkingDirectory = ".";
		spec.CommandLineArgs = args;

		spec.MinGPURequirements.IsDiscreteGPU = true;

		return new Meadow(spec);
	}

}

