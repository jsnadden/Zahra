#pragma once

#include "Zahra/Renderer/Camera.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/MouseEvent.h"

namespace Zahra
{

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(float dt);
		void OnEvent(Event& event);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& event);
		bool OnWindowResized(WindowResizedEvent& event);

		bool m_EnableRotation;

		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;

		OrthographicCamera m_Camera;

		glm::vec3 m_Position;
		float m_Rotation;

		glm::vec3 m_Velocity;
		float m_AngularVelocity;

		float m_TranslationSpeed = 0.5f;
		float m_RotationSpeed = 0.5f;
		float m_Inertia = 0.89f;
	};

}

