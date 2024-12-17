#include <Zahra.h>
#include <Zahra/Core/EntryPoint.h>

#include "Sandbox.h"

class Sandbox : public Zahra::Application
{
public:

	Sandbox(const Zahra::ApplicationSpecification& spec)
		: Zahra::Application(spec)
	{
		PushLayer(new SandboxLayer());
	}

	~Sandbox()
	{

	}


};

Zahra::Application* Zahra::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.Version = ApplicationVersion(0,1,0);
	spec.WorkingDirectory = ".";
	spec.CommandLineArgs = args;

	spec.RendererConfig.ForceShaderCompilation = false;
	spec.RendererConfig.MaximumBoundTextures = 32;

	spec.GPURequirements.IsDiscreteGPU = true;
	spec.GPURequirements.AnisotropicFiltering = true;
	spec.GPURequirements.MinBoundTextureSlots = 32;

	spec.ImGuiConfig.Enabled = false;

	return new Sandbox(spec);
}
