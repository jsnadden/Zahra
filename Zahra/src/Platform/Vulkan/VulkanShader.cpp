#include "zpch.h"
#include "VulkanShader.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Zahra/Core/Timer.h"
#include "Zahra/Utils/FileIO.h"

#include <fstream>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

namespace Zahra
{
	namespace VulkanUtils
	{
		static std::filesystem::path s_CacheDirectory = "Cache/Shaders/Vulkan";

		static const std::filesystem::path& GetSPIRVCachePath()
		{
			if (!std::filesystem::exists(s_CacheDirectory))
			{
				std::filesystem::create_directories(s_CacheDirectory);
			}

			return s_CacheDirectory;
		}

		static ShaderStageBits ShaderStageToStageBit(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:
				{
					return ShaderStageBits::VertexBit;
				}

				case ShaderStage::TesselationControl:
				{
					return ShaderStageBits::TesselationControlBit;
				}

				case ShaderStage::TesselationEvaluation:
				{
					return ShaderStageBits::TesselationEvaluationBit;
				}

				case ShaderStage::Geometry:
				{
					return ShaderStageBits::GeometryBit;
				}

				case ShaderStage::Fragment:
				{
					return ShaderStageBits::FragmentBit;
				}

				case ShaderStage::Compute:
				{
					return ShaderStageBits::ComputeBit;
				}

				default:
				{
					Z_CORE_ASSERT(false, "Unrecognised shader stage");
					break;
				}
			}
		}

		static const char* ShaderStageToFileExtension(ShaderStage stage)
		{
			const char* extension;

			switch (stage)
			{
				case ShaderStage::Vertex:
				{
					extension = "vert";
					break;
				}

				case ShaderStage::TesselationControl:
				{
					extension = "tesc";
					break;
				}

				case ShaderStage::TesselationEvaluation:
				{
					extension = "tese";
					break;
				}

				case ShaderStage::Geometry:
				{
					extension = "geom";
					break;
				}

				case ShaderStage::Fragment:
				{
					extension = "frag";
					break;
				}

				case ShaderStage::Compute:
				{
					extension = "comp";
					break;
				}

				default:
				{
					Z_CORE_ASSERT(false, "Unrecognised shader stage");
					break;
				}
			}

			return extension;
		}

		static std::string ShaderStageToString(ShaderStage stage)
		{
			std::string stageName;

			switch (stage)
			{
				case ShaderStage::Vertex:
				{
					stageName = "vertex";
					break;
				}

				case ShaderStage::TesselationControl:
				{
					stageName = "tesselation control";
					break;
				}

				case ShaderStage::TesselationEvaluation:
				{
					stageName = "tesselation evaluation";
					break;
				}

				case ShaderStage::Geometry:
				{
					stageName = "geometry";
					break;
				}

				case ShaderStage::Fragment:
				{
					stageName = "fragment";
					break;
				}

				case ShaderStage::Compute:
				{
					stageName = "compute";
					break;
				}

				default:
				{
					Z_CORE_ASSERT(false, "Unrecognised shader stage");
					break;
				}
			}

			return stageName;
		}

		static shaderc_shader_kind ShaderStageToShaderC(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:
				{
					return shaderc_vertex_shader;
				}

				case ShaderStage::TesselationControl:
				{
					return shaderc_tess_control_shader;
				}

				case ShaderStage::TesselationEvaluation:
				{
					return shaderc_tess_evaluation_shader;
				}

				case ShaderStage::Geometry:
				{
					return shaderc_geometry_shader;
				}

				case ShaderStage::Fragment:
				{
					return shaderc_fragment_shader;
				}

				case ShaderStage::Compute:
				{
					return shaderc_compute_shader;
				}

				default:
				{
					Z_CORE_ASSERT(false, "Unrecognised shader stage");
					break;
				}
			}
		}

		static VkShaderStageFlagBits ShaderStageToVkFlagBit(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:
				{
					return VK_SHADER_STAGE_VERTEX_BIT;
				}

				case ShaderStage::TesselationControl:
				{
					return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				}

				case ShaderStage::TesselationEvaluation:
				{
					return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				}

				case ShaderStage::Geometry:
				{
					return VK_SHADER_STAGE_GEOMETRY_BIT;
				}

				case ShaderStage::Fragment:
				{
					return VK_SHADER_STAGE_FRAGMENT_BIT;
				}

				case ShaderStage::Compute:
				{
					return VK_SHADER_STAGE_COMPUTE_BIT;
				}

				default:
				{
					Z_CORE_ASSERT(false, "Unrecognised shader stage");
					break;
				}
			}
		}
	}

	VulkanShader::VulkanShader(ShaderSpecification& specification)
		: m_Specification(specification)
	{
		bool loaded = true;

		// TODO: include other shader stages
		if (specification.StageBitMask & ShaderStageBits::VertexBit)
			loaded &= ReadShaderSource(ShaderStage::Vertex);

		if (specification.StageBitMask & ShaderStageBits::FragmentBit)
			loaded &= ReadShaderSource(ShaderStage::Fragment);
	
		if (!loaded)
		{
			std::string errorMessage = "Failed to load source code for shader '{0}'" + specification.Name;
			throw std::runtime_error(errorMessage);
		}

		std::filesystem::path cacheDirectory = VulkanUtils::GetSPIRVCachePath();
		
		Timer shaderCreationTimer;
		{
			CompileOrGetSPIRV(m_SPIRVBytecode, cacheDirectory, false);
			CompileOrGetSPIRV(m_SPIRVBytecode_Debug, cacheDirectory, true);
			Reflect();
			CreateModules();
		}
		Z_CORE_TRACE("Shader creation took {0} ms", shaderCreationTimer.ElapsedMillis());

	}

	VulkanShader::~VulkanShader()
	{
		for (auto& stageInfo : m_PipelineShaderStageInfos)
		{
			vkDestroyShaderModule(VulkanContext::Get()->GetDevice()->LogicalDevice, stageInfo.module, nullptr);
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

	void VulkanShader::CompileOrGetSPIRV(std::unordered_map<ShaderStage, std::vector<uint32_t>>& bytecode, const std::filesystem::path& cacheDirectory, bool debug)
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

			if (filestream.is_open())
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
				auto& buffer = m_ReflectionData[stage].UniformBuffers.emplace_back();

				const auto& bufferType = compiler.get_type(resource.base_type_id);

				buffer.Name = resource.name;
				buffer.ByteSize = compiler.get_declared_struct_size(bufferType);
				buffer.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				buffer.MemberCount = bufferType.member_types.size();
			}
		}

	}

	void VulkanShader::CreateModules()
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->LogicalDevice;
		
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

	void VulkanShader::Bind() const
	{
	}

	void VulkanShader::Unbind() const
	{
	}

	std::string VulkanShader::GetSourceFilename(ShaderStage stage)
	{
		return m_Specification.Name + "." + VulkanUtils::ShaderStageToFileExtension(stage);
	}
}
