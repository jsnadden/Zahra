#pragma once

#include "Camera.h"
#include "Zahra/Events/Event.h"
#include "Zahra/Events/MouseEvent.h"

#include <glm/glm.hpp>

namespace Zahra
{
	class EditorCamera : Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(float dt);
		void OnEvent(Event& e);

		inline float GetFocalDistance() const { return m_FocalDistance; }
		inline void SetFocalDistance(float distance) { m_FocalDistance = distance; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetProjection() const { return m_Projection; }
		glm::mat4 GetPVMatrix() const { return m_Projection * m_ViewMatrix; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }

		void LockRotation(bool lock) { m_RotationLocked = lock; }
		bool RotationLocked() { return m_RotationLocked; }

		void ResetCamera();

	private:
		float m_FOV = .79f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_LastMousePosition = { 0.0f, 0.0f };

		float m_FocalDistance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		bool m_RotationLocked = false;

		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);
		void MousePush(float delta);

		glm::vec3 CalculatePosition() const;

		glm::vec2 PanVelocity() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	};


}


