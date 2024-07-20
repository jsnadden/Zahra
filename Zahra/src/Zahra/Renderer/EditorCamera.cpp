#include "zpch.h"
#include "EditorCamera.h"

#include "Zahra/Core/Input.h"
#include "Zahra/Core/KeyCodes.h"
#include "Zahra/Core/MouseCodes.h"

#include <glfw/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Zahra
{

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(glm::perspective(fov, aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		if (m_RotationLocked) m_Yaw = m_Pitch = 0.0f;
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::inverse(glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation));
	}

	glm::vec2 EditorCamera::PanVelocity() const
	{
		// some quadratic voodoo for smooth motion

		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 1.f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_FocalDistance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = 2.0f * distance;//distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate(float dt)
	{
		if (Input::IsKeyPressed(Key::LeftAlt))
		{
			const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			glm::vec2 delta = (mouse - m_LastMousePosition) * 0.003f;
			m_LastMousePosition = mouse;

			if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
				MouseRotate(delta);
		}

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrolledEvent>(Z_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& event)
	{
		float delta = event.GetOffsetY() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		glm::vec2& velocity = PanVelocity();
		// For x, the camera pans in the opposite direction to mouse displacement (as usual), hence the -ve
		m_FocalPoint += -GetRightDirection() * delta.x * velocity.x * m_FocalDistance;
		// For y, the delta comes from screenspace coords, which have y pointing downwards, so we end up with a +ve
		m_FocalPoint += GetUpDirection() * delta.y * velocity.y * m_FocalDistance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw -= yawSign * delta.x * RotationSpeed();
		m_Pitch -= delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_FocalDistance -= delta * ZoomSpeed();

		/*if (m_FocalDistance < 1.0f) // this is a weird and upsetting choice. the focal point suddenly zooms forward
		{
			m_FocalPoint += GetForwardDirection();
			m_FocalDistance = 1.0f;
		}*/
	}

	void EditorCamera::MousePush(float delta)
	{
		m_FocalPoint += -GetForwardDirection() * delta * 3.f;
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_FocalDistance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(m_Pitch, m_Yaw, 0.0f));
	}

	void EditorCamera::ResetCamera()
	{
		// TODO: return to initial focus/orientation
	}

}
