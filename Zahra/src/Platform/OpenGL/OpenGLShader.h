#pragma once

#include "Zahra/Renderer/Shader.h"

#include <glm/glm.hpp>

// TODO: this can/should be removed, and instead just include glad.h here, but for now we're doing some naughty casting in sandbox
typedef unsigned int GLenum;

namespace Zahra
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const std::string& GetName() const override { return m_Name; }

		virtual void SetInt(const  std::string& name, int value) override;
		virtual void SetIntArray(const  std::string& name, uint32_t count, int* values) override;
		
		virtual void SetFloat(const  std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& values) override;
		virtual void SetFloat3(const std::string& name, const glm::vec4& values) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& values) override;
		
		virtual void SetMat2(const std::string& name, const glm::mat2& matrix) override;
		virtual void SetMat3(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& matrix) override;
		

	private:
		uint32_t m_RendererID;
		std::string m_Name;

		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> ParseShaderSrc(std::string& shaderSource);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

	};
}