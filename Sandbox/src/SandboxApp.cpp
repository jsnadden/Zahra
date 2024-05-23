#include <Zahra.h>

class Sandbox : public Zahra::Application
{
public:

	Sandbox()
	{

	}

	~Sandbox()
	{

	}


};

Zahra::Application* Zahra::CreateApplication()
{
	return new Sandbox();
}

