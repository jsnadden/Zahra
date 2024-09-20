#pragma once

#include "Zahra/Renderer/Shader.h"

#include <filesystem>
#include <vulkan/vulkan.h>

namespace Zahra
{


	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& name, const std::filesystem::path& directory);
		~VulkanShader();

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
		std::filesystem::path m_SourceDirectory;

		std::unordered_map<Shader::Stage, std::string> m_GLSLSource;
		std::unordered_map<Shader::Stage, std::vector<uint32_t>> m_SPIRVBytecode;
		std::unordered_map<Shader::Stage, VkShaderModule> m_Modules;

		bool ReadShaderSource(Shader::Stage stage);
		void CompileOrGetSPIRV(const std::unordered_map<Shader::Stage, std::string>& shaderSources, const std::filesystem::path& cacheDirectory);
		void CreateModules(const std::unordered_map<Shader::Stage, std::vector<uint32_t>>& bytecode);

		std::string GetSourceFilename(Shader::Stage stage);
	};

}

