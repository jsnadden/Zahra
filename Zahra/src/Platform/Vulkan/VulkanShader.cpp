#include "zpch.h"
#include "VulkanShader.h"

#include "Zahra/Utils/FileIO.h"

namespace Zahra
{
	// TODO: currently only dealing with vertex and fragment
	// shaders, others will be included as I build on this.
	VulkanShader::VulkanShader(const std::string& name, const std::filesystem::path& directory)
		: m_Name(name), m_SourceDirectory(directory)
	{
		std::filesystem::path vertexSource = directory / (name + ".vert");
		std::filesystem::path fragmentSource = directory / (name + ".frag");

		m_GLSLSource[VK_SHADER_STAGE_VERTEX_BIT] = FileIO::ReadBytes(vertexSource);
		m_GLSLSource[VK_SHADER_STAGE_FRAGMENT_BIT] = FileIO::ReadBytes(fragmentSource);

	}

	void VulkanShader::CompileOrGetBinaries(const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources)
	{

	}

	void VulkanShader::CreateProgram()
	{

	}

	void VulkanShader::Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData)
	{

	}


}
