#include "zpch.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Zahra {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = glm::ortho(left, right, top, bottom, -1.0f, 1.0f);
		m_ViewMatrix = glm::mat4(1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

		m_Position = glm::vec3(0.0f);
		m_Rotation = 0.0f;
	}

	void OrthographicCamera::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		RecalculateMatrices();
	}

	void OrthographicCamera::SetRotation(const float rotation)
	{
		m_Rotation = rotation;
		RecalculateMatrices();
	}

	void OrthographicCamera::RecalculateMatrices()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}



}