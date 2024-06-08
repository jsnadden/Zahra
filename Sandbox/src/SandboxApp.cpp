#include <Zahra.h>

class ExampleLayer : public Zahra::Layer
{
public:
	ExampleLayer() : Layer("Example_Layer") {}

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
		: Zahra::Application()
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