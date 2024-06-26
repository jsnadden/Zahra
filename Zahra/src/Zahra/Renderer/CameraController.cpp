#include "zpch.h"
#include "CameraController.h"

#include "Zahra/Core/Input.h"
#include "Zahra/Core/KeyCodes.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Zahra
{
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool enableRotation)
		: m_AspectRatio(aspectRatio), m_EnableRotation(enableRotation),
		m_Velocity(0.0f), m_AngularVelocity(0.0f),
		m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
	{
		m_Position = m_Camera.GetPosition();
		m_Rotation = m_Camera.GetRotation();
	}

	void OrthographicCameraController::OnUpdate(float dt)
	{
		// Reset camera
		if (Zahra::Input::IsKeyPressed(Z_KEY_SPACE))
		{
			m_Velocity *= .0f;
			m_AngularVelocity *= .0f;

			m_Camera.SetPosition(glm::vec3(.0f));
			m_Camera.SetRotation(.0f);

			m_ZoomLevel = 1.0f;
			m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		}

		// Cache camera data
		m_Position = m_Camera.GetPosition();
		m_Rotation = m_Camera.GetRotation();

		// Inertia
		m_Velocity *= m_Inertia;
		m_AngularVelocity *= m_Inertia;

		// Move camera
		int dir = 1;

		if (Input::IsKeyPressed(Z_KEY_W))
		{
			dir = 1;
			m_Velocity += glm::rotate(glm::mat4(1.0f), m_Rotation,
				glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(.0f, 1.0f, .0f, 1.0f);
		}
		if (Input::IsKeyPressed(Z_KEY_S))
		{
			dir = -1;
			m_Velocity += glm::rotate(glm::mat4(1.0f), m_Rotation,
				glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(.0f, -1.0f, .0f, 1.0f);
		}
		if (Input::IsKeyPressed(Z_KEY_D))
		{
			m_Velocity += glm::rotate(glm::mat4(1.0f), m_Rotation,
				glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(1.0f, .0f, .0f, 1.0f);
		}
		if (Input::IsKeyPressed(Z_KEY_A))
		{
			m_Velocity += glm::rotate(glm::mat4(1.0f), m_Rotation,
				glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(-1.0f, .0f, .0f, 1.0f);
		}

		m_Camera.SetPosition(m_Position + m_ZoomLevel * m_TranslationSpeed * dt * m_Velocity);

		if (m_EnableRotation)
		{
			if (Input::IsKeyPressed(Z_KEY_Q))
				m_AngularVelocity += 1.0f;

			if (Input::IsKeyPressed(Z_KEY_E))
				m_AngularVelocity -= 1.0f;

			m_Camera.SetRotation(m_Rotation + dir * m_RotationSpeed * dt * m_AngularVelocity);
		}

	}

	void OrthographicCameraController::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<MouseScrolledEvent>(Z_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizedEvent>(Z_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& event)
	{
		m_ZoomLevel *= glm::exp(glm::log(.8f) * event.GetOffsetY());
		m_ZoomLevel = std::clamp(m_ZoomLevel, .05f, 20.0f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizedEvent& event)
	{
		m_AspectRatio = ( (float)event.GetWidth() ) / ( (float)event.GetHeight() );
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

}


