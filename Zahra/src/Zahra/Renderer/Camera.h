#pragma once

#include <glm/glm.hpp>

namespace Zahra {

	// TODO: if the only use of Camera is the subclass SceneCamera, just fold this into that class
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_Projection(projection) {}
		
		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }
		
	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};


	// TODO: remove this
	class OrthographicCamera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetPosition(const glm::vec3& position);
		const glm::vec3& GetPosition() const { return m_Position; }

		void SetRotation(float rotation);
		float GetRotation() const { return m_Rotation; }

		void SetProjection(float left, float right, float bottom, float top);

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetPVMatrix() const { return m_PVMatrix; }
		
	private:
		void RecalculateMatrices();

		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_PVMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;
	};

}