#include <Zahra.h>
#include <Zahra/Core/EntryPoint.h>

#include "Sandbox2D.h"

class Sandbox : public Zahra::Application
{
public:

	Sandbox()
		: Zahra::Application("Sandbox App")
	{
		PushLayer(new Sandbox2DLayer());
	}

	~Sandbox()
	{

	}


};

Zahra::Application* Zahra::CreateApplication()
{
	return new Sandbox();
}
