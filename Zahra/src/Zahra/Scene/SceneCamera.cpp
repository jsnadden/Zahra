#include "zpch.h"
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Zahra
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetOrthographicSize(float size)
	{
		m_OrthographicSize = size;

		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(float width, float height)
	{
		m_AspectRatio = width / height;

		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		m_Projection = glm::ortho(m_OrthographicSize * m_AspectRatio * -.5f,
			m_OrthographicSize * m_AspectRatio * .5f,
			m_OrthographicSize * -.5f,
			m_OrthographicSize * .5f,
			m_OrthographicNear,
			m_OrthographicFar);
	}

}

