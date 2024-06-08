#include <Zahra.h>

#include "imgui/imgui.h"

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

	void OnImGuiRender() override
	{
		ImGui::Begin("Testeze");
		ImGui::Text("Peenussss");
		ImGui::End();

	}
};

class Sandbox : public Zahra::Application
{
public:

	Sandbox()
		: Zahra::Application()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}


};

Zahra::Application* Zahra::CreateApplication()
{
	return new Sandbox();
}