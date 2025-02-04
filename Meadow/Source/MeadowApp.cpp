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
		// TODO: maybe move some of this to project config
		ApplicationSpecification spec;
		spec.Name = "Meadow";
		spec.Version = ApplicationVersion(0, 1, 0);
		spec.WorkingDirectory = ".";
		spec.CommandLineArgs = args;
		spec.IsEditor = true;

		spec.ShaderSourceDirectory = "./Resources/Shaders";
		spec.ShaderCacheDirectory = "./Cache/Shaders";

		spec.RendererConfig.DesiredFramesInFlight = 3;
		spec.RendererConfig.ForceShaderRecompilation = false;

		spec.GPURequirements.IsDiscreteGPU = true;
		spec.GPURequirements.AnisotropicFiltering = true;
		spec.GPURequirements.MinBoundTextureSlots = 32;

		spec.ImGuiConfig.Enabled = true;

		return new Meadow(spec);
	}

}

