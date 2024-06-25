#pragma once

#include <glm/glm.hpp>

namespace Zahra {

	class Camera
	{
	public:
		virtual const glm::mat4& GetProjectionMatrix() const = 0;
		virtual const glm::mat4& GetViewMatrix() const = 0;
		virtual const glm::mat4& GetPVMatrix() const = 0;
	};

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetPosition(const glm::vec3& position);
		const glm::vec3& GetPosition() const { return m_Position; }

		void SetRotation(float rotation);
		float GetRotation() const { return m_Rotation; }

		void SetProjection(float left, float right, float bottom, float top);

		const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		const glm::mat4& GetPVMatrix() const override { return m_PVMatrix; }
		
	private:
		void RecalculateMatrices();

		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_PVMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;
	};

}