#pragma once

#include "Zahra/Layer.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/KeyEvent.h"
#include "Zahra/Events/MouseEvent.h"

namespace Zahra
{
	class ZAHRA_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach();
		void OnDetach();

		void OnUpdate();
		void OnEvent(Event& event);

	private:
		float m_Time = 0.0f;

		// Handle mouse events
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
		bool OnMouseMovedEvent(MouseMovedEvent& e);
		bool OnMouseScrolledEvent(MouseScrolledEvent& e);
		// Handle keyboard events
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnKeyReleasedEvent(KeyReleasedEvent& e);
		bool OnKeyTypedEvent(KeyTypedEvent& e);
		// Handle window events
		bool OnWindowResizeEvent(WindowResizeEvent& e);
		bool OnWindowCloseEvent(WindowCloseEvent& e);
		//// Handle app events
		//bool OnAppTickEvent(AppTickEvent& e);
		//bool OnAppUpdateEvent(AppUpdateEvent& e);
		//bool OnAppRenderEvent(AppRenderEvent& e);

	};
}