#pragma once

#include "Zahra/Renderer/Camera.h"

#include <glm/glm.hpp>

namespace Zahra
{

	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetOrthographic(float size, float nearClip, float farClip);

		void SetOrthographicSize(float size);
		float GetOrthographicSize() { return m_OrthographicSize; }

		void SetViewportSize(float width, float height);

	private:
		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;
		float m_AspectRatio = 1.0f;

		void RecalculateProjection();

	};

}

