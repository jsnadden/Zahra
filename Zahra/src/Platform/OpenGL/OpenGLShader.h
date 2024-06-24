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

		void UploadUniformInt(const  std::string& name, int value);
		
		void UploadUniformFloat(const  std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec4& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		
		void UploadUniformMat2(const std::string& name, const glm::mat2& matrix);
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		

	private:
		uint32_t m_RendererID;
		std::string m_Name;

		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> ParseShaderSrc(std::string& shaderSource);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

	};
}