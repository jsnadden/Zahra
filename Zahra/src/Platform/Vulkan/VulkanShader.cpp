#include "zpch.h"
#include "VulkanShader.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanShaderUtils.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Zahra/Core/Timer.h"
#include "Zahra/Utils/FileIO.h"

#include <fstream>


namespace Zahra
{
	

	VulkanShader::VulkanShader(ShaderSpecification& specification, bool forceCompile)
		: m_Specification(specification)
	{
		bool loaded = true;

		// TODO: include other shader stages
		if (specification.StageBitMask & ShaderStageBits::VertexStageBit)
			loaded &= ReadShaderSource(ShaderStage::Vertex);

		if (specification.StageBitMask & ShaderStageBits::FragmentStageBit)
			loaded &= ReadShaderSource(ShaderStage::Fragment);
	
		if (!loaded)
		{
			std::string errorMessage = "Failed to load source code for shader '{0}'" + specification.Name;
			throw std::runtime_error(errorMessage);
		}

		std::filesystem::path cacheDirectory = VulkanUtils::GetSPIRVCachePath();
		
		Timer shaderCreationTimer;
		{
			CompileOrGetSPIRV(m_SPIRVBytecode, cacheDirectory, false, forceCompile);
			CompileOrGetSPIRV(m_SPIRVBytecode_Debug, cacheDirectory, true, forceCompile);
			CreateModules();
		}
		Z_CORE_TRACE("Shader creation took {0} ms", shaderCreationTimer.ElapsedMillis());

		Reflect();
		CreateDescriptorSetLayouts();
	}

	VulkanShader::~VulkanShader()
	{
		VkDevice& device = VulkanContext::Get()->GetDevice()->GetVkDevice();

		for (auto& stageInfo : m_PipelineShaderStageInfos)
		{
			vkDestroyShaderModule(device, stageInfo.module, nullptr);
		}

		for (auto& layout : m_DescriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(device, layout, nullptr);
		}

	}

	bool VulkanShader::ReadShaderSource(ShaderStage stage)
	{
		std::filesystem::path sourceFilepath = m_Specification.SourceDirectory / GetSourceFilename(stage);

		if (!std::filesystem::exists(sourceFilepath))
		{
			std::string errorMessage = "Shader source file '" + sourceFilepath.string() + "' does not exist";
			Z_CORE_ASSERT(false, errorMessage.c_str());
		}

		uint32_t sourceCodeSize;
		std::string sourceCode = FileIO::ReadAsString(sourceFilepath, &sourceCodeSize);
		if (sourceCodeSize)
		{
			m_GLSLSource[stage] = sourceCode;
			Z_CORE_TRACE("Successfully loaded shader source file '{0}'", sourceFilepath.string().c_str());
		}
		else
		{
			return false;
		}

		return true;
	}

	void VulkanShader::CompileOrGetSPIRV(std::unordered_map<ShaderStage, std::vector<uint32_t>>& bytecode, const std::filesystem::path& cacheDirectory, bool debug, bool forceCompile)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		if (debug) 
		{
			options.SetGenerateDebugInfo();
			options.SetOptimizationLevel(shaderc_optimization_level_zero);
		}
		else
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		bytecode.clear();

		for (auto&& [stage, source] : m_GLSLSource)
		{

			std::filesystem::path spirvFilename = m_Specification.Name + "_" + VulkanUtils::ShaderStageToFileExtension(stage);
			if (debug) spirvFilename += "_debug";
 			spirvFilename += ".spv";
			std::filesystem::path spirvFilepath = cacheDirectory / spirvFilename;

			std::ifstream filestream(spirvFilepath, std::ios::in | std::ios::binary | std::ios::ate);

			if (filestream.is_open() && !forceCompile)
			{
				auto size = filestream.tellg();

				filestream.seekg(0, std::ios::beg);
				auto& bin = bytecode[stage];
				bin.resize(size / sizeof(uint32_t));
				filestream.read((char*)bin.data(), size);
			}
			else
			{
				std::filesystem::path sourceFilepath = m_Specification.SourceDirectory / GetSourceFilename(stage);

				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source,
					VulkanUtils::ShaderStageToShaderC(stage), sourceFilepath.string().c_str(), options);

				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					Z_CORE_ERROR(result.GetErrorMessage());
					Z_CORE_ASSERT(false, "Vulkan shader failed to compile");
				}

				bytecode[stage] = std::vector<uint32_t>(result.cbegin(), result.cend());

				std::ofstream out(spirvFilepath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = bytecode[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}

		}

	}

	void VulkanShader::Reflect()
	{
		for (auto&& [stage, data] : m_SPIRVBytecode_Debug)
		{
			spirv_cross::Compiler compiler(data);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();

			for (const auto& resource : resources.uniform_buffers)
			{
				auto& bufferData = m_ReflectionData.ResourceMetadata.emplace_back();

				const auto& bufferType = compiler.get_type(resource.base_type_id);

				uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				if (set > m_ReflectionData.MaxSetIndex) m_ReflectionData.MaxSetIndex = set;

				bufferData.Name = resource.name;
				bufferData.Type = ShaderResourceType::UniformBuffer;
				bufferData.Set = set;
				bufferData.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				bufferData.Stage = stage;
				bufferData.ByteSize = compiler.get_declared_struct_size(bufferType);
				bufferData.MemberCount = bufferType.member_types.size();

				if (!bufferType.array.empty()) bufferData.ArrayLength = bufferType.array[0]; // TODO: account for multidimensional (i.e. nested) arrays?

			}

			// TODO: additional for loops for other shader resources. For details see:
			// https://github.com/KhronosGroup/SPIRV-Cross/wiki/Reflection-API-user-guide

		}

	}

	void VulkanShader::CreateDescriptorSetLayouts()
	{
		uint32_t setCount = m_ReflectionData.MaxSetIndex + 1;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> layoutBindings(setCount);

		for (auto& bufferData : m_ReflectionData.ResourceMetadata)
		{
			auto& layoutBinding = layoutBindings[bufferData.Set].emplace_back();

			layoutBinding.binding = bufferData.Binding;
			layoutBinding.stageFlags = VulkanUtils::ShaderStageToVkFlagBit(bufferData.Stage);
			layoutBinding.descriptorType = VulkanUtils::ShaderResourceTypeToVkDescriptorType(bufferData.Type);
			layoutBinding.descriptorCount = bufferData.ArrayLength;
			layoutBinding.pImmutableSamplers = nullptr;
		}

		m_DescriptorSetLayouts.resize(setCount);

		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		for (int i = 0; i < setCount; i++)
		{
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = layoutBindings[i].size();
			layoutInfo.pBindings = layoutBindings[i].data();

			VulkanUtils::ValidateVkResult(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayouts[i]));
		}

	}

	void VulkanShader::CreateModules()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		
		for (const auto& [stage, bytes] : m_SPIRVBytecode)
		{
			VkPipelineShaderStageCreateInfo& shaderStageInfo = m_PipelineShaderStageInfos.emplace_back();
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = VulkanUtils::ShaderStageToVkFlagBit(stage);
			shaderStageInfo.pName = "main";

			VkShaderModuleCreateInfo moduleInfo{};
			moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleInfo.codeSize = bytes.size() * sizeof(uint32_t);
			moduleInfo.pCode = bytes.data();

			VulkanUtils::ValidateVkResult(vkCreateShaderModule(device, &moduleInfo, nullptr, &shaderStageInfo.module));
		}
	}


	std::string VulkanShader::GetSourceFilename(ShaderStage stage)
	{
		return m_Specification.Name + "." + VulkanUtils::ShaderStageToFileExtension(stage);
	}
}
