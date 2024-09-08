#include <Zahra.h>
#include <Zahra/Core/EntryPoint.h>

#include "Sandbox2D.h"

class Sandbox : public Zahra::Application
{
public:

	Sandbox(const Zahra::ApplicationSpecification& spec)
		: Zahra::Application(spec)
	{
		PushLayer(new Sandbox2DLayer());
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
	spec.WorkingDirectory = "..\\Meadow";
	spec.CommandLineArgs = args;

	return new Sandbox(spec);
}
