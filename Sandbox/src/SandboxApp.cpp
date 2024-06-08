#include <Zahra.h>

class ExampleLayer : public Zahra::Layer
{
public:
	ExampleLayer() : Layer("E.g. lyr") {}

	void OnUpdate() override
	{
		
	}

	void OnEvent(Zahra::Event& event) override
	{
		//Z_TRACE(event);
	}
};

class Sandbox : public Zahra::Application
{
public:

	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Zahra::ImGuiLayer());
	}

	~Sandbox()
	{

	}


};

Zahra::Application* Zahra::CreateApplication()
{
	return new Sandbox();
}