#pragma once

#include "Zahra/ImGui/ImGuiLayer.h"

namespace Zahra
{
	class OpenGLImGuiLayer : public ImGuiLayer
	{
	public:
		OpenGLImGuiLayer();
		OpenGLImGuiLayer(const std::string& name);
		virtual ~OpenGLImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		virtual void Begin() override;
		virtual void End() override;

	};

}

