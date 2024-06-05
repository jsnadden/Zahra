#include <Zahra.h>

//class ExampleLayer : public Zahra::Layer
//{
//public:
//	ExampleLayer() : Layer("E.g. lyr") {}
//
//	void OnUpdate() override
//	{
//		Z_INFO("ExampleLayer::Update");
//	}
//
//	void OnEvent(Zahra::Event& event) override
//	{
//		Z_TRACE(event);
//	}
//};

class Sandbox : public Zahra::Application
{
public:

	Sandbox()
	{
		//PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}


};

Zahra::Application* Zahra::CreateApplication()
{
	return new Sandbox();
}