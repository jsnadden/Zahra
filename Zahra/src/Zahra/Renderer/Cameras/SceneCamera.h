#pragma once

#include "Zahra/Renderer/Cameras/Camera.h"

#include <glm/glm.hpp>

namespace Zahra
{

	struct OrthographicCameraData
	{
		float Size = 10.0f;

		float Near = -1.0f, Far = 1.0f;
	};

	struct PerspectiveCameraData
	{
		float VerticalFOV = glm::radians(60.0f);

		float Near = .01f, Far = 1000.0f;
	};

	class SceneCamera : public Camera
	{
	public:

		enum class ProjectionType { Orthographic = 0, Perspective = 1 };

		SceneCamera();
		virtual ~SceneCamera() override = default;

		void SetViewportSize(float width, float height);

		void SetProjectionType(ProjectionType type);
		ProjectionType& GetProjectionType() { return m_ProjectionType; }

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Orthographic data
		void SetOrthographicData(float size, float nearClip, float farClip);

		void SetOrthographicSize(float size);
		float GetOrthographicSize() const { return m_OrthographicData.Size; }
		
		void SetOrthographicNearClip(float nearClip);
		float GetOrthographicNearClip() const { return m_OrthographicData.Near; }
		
		void SetOrthographicFarClip(float farClip);
		float GetOrthographicFarClip() const { return m_OrthographicData.Far; }
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Perspective data
		void SetPerspectiveData(float fov, float nearClip, float farClip);

		void SetPerspectiveFOV(float fov);
		float GetPerspectiveFOV() const { return m_PerspectiveData.VerticalFOV; }

		void SetPerspectiveNearClip(float nearClip);
		float GetPerspectiveNearClip() const { return m_PerspectiveData.Near; }

		void SetPerspectiveFarClip(float farClip);
		float GetPerspectiveFarClip() const { return m_PerspectiveData.Far; }
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		OrthographicCameraData m_OrthographicData;
		PerspectiveCameraData m_PerspectiveData;

		float m_AspectRatio = 1.0f;

		void RecalculateProjection();

	};

}

