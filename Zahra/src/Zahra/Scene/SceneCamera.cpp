#include "zpch.h"
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Zahra
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographicData(float size, float nearClip, float farClip)
	{
		m_OrthographicData.Size = size;
		m_OrthographicData.Near = nearClip;
		m_OrthographicData.Far = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetOrthographicSize(float size)
	{
		m_OrthographicData.Size = size;

		RecalculateProjection();
	}

	void SceneCamera::SetOrthographicNearClip(float nearClip)
	{
		m_OrthographicData.Near = nearClip;

		RecalculateProjection();
	}

	void SceneCamera::SetOrthographicFarClip(float farClip)
	{
		m_OrthographicData.Far = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetPerspectiveData(float fov, float nearClip, float farClip)
	{
		m_PerspectiveData.VerticalFOV = fov;
		m_PerspectiveData.Near = nearClip;
		m_PerspectiveData.Far = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetPerspectiveFOV(float fov)
	{
		m_PerspectiveData.VerticalFOV = fov;

		RecalculateProjection();
	}

	void SceneCamera::SetPerspectiveNearClip(float nearClip)
	{
		m_PerspectiveData.Near = nearClip;

		RecalculateProjection();
	}

	void SceneCamera::SetPerspectiveFarClip(float farClip)
	{
		m_PerspectiveData.Far = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(float width, float height)
	{
		Z_CORE_ASSERT(width > 0 && height > 0);

		m_AspectRatio = width / height;

		RecalculateProjection();
	}

	void SceneCamera::SetProjectionType(ProjectionType type)
	{
		m_ProjectionType = type;

		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		switch (m_ProjectionType)
		{
		case ProjectionType::Orthographic:
		{
			m_Projection = glm::ortho(m_OrthographicData.Size * m_AspectRatio * -.5f,
				m_OrthographicData.Size * m_AspectRatio * .5f,
				m_OrthographicData.Size * -.5f,
				m_OrthographicData.Size * .5f,
				m_OrthographicData.Near,
				m_OrthographicData.Far);
			break;
		}
		case ProjectionType::Perspective:
		{
			m_Projection = glm::perspective(m_PerspectiveData.VerticalFOV,
				m_AspectRatio,
				m_PerspectiveData.Near,
				m_PerspectiveData.Far);
			break;
		}
		default: Z_CORE_ASSERT(false, "Invalid camera projection type");
		}

	}

}

