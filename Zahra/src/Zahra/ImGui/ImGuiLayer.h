#pragma once

#include "Zahra/Core/Layer.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/KeyEvent.h"
#include "Zahra/Events/MouseEvent.h"

namespace Zahra
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetColourTheme();

	private:
		bool m_BlockEvents = true;

	};
}