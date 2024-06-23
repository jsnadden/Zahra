#pragma once

#include "Zahra/Renderer/Shader.h"

#include <glm/glm.hpp>

// TODO: this can/should be removed, and instead just include glad.h here, but for now we're doing some casting in sandbox
typedef unsigned int GLenum;

namespace Zahra
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;


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

		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> ParseShaderSrc(std::string& shaderSrc);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSrcs);

	};
}