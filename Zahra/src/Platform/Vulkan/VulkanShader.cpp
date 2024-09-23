#include "zpch.h"
#include "VulkanShader.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Zahra/Core/Timer.h"
#include "Zahra/Utils/FileIO.h"

#include <fstream>
#include <shaderc/shaderc.hpp>

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

		static Shader::StageBits ShaderStageToStageBit(Shader::Stage stage)
		{
			switch (stage)
			{
				case Shader::Stage::Vertex:
				{
					return Shader::StageBits::VertexBit;
				}

				case Shader::Stage::TesselationControl:
				{
					return Shader::StageBits::TesselationControlBit;
				}

				case Shader::Stage::TesselationEvaluation:
				{
					return Shader::StageBits::TesselationEvaluationBit;
				}

				case Shader::Stage::Geometry:
				{
					return Shader::StageBits::GeometryBit;
				}

				case Shader::Stage::Fragment:
				{
					return Shader::StageBits::FragmentBit;
				}

				case Shader::Stage::Compute:
				{
					return Shader::StageBits::ComputeBit;
				}

				default:
				{
					Z_CORE_ASSERT(false, "Unrecognised shader stage");
					break;
				}
			}
		}

		static const char* ShaderStageToFileExtension(Shader::Stage stage)
		{
			const char* extension;

			switch (stage)
			{
				case Shader::Stage::Vertex:
				{
					extension = "vert";
					break;
				}

				case Shader::Stage::TesselationControl:
				{
					extension = "tesc";
					break;
				}

				case Shader::Stage::TesselationEvaluation:
				{
					extension = "tese";
					break;
				}

				case Shader::Stage::Geometry:
				{
					extension = "geom";
					break;
				}

				case Shader::Stage::Fragment:
				{
					extension = "frag";
					break;
				}

				case Shader::Stage::Compute:
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

		static std::string ShaderStageToString(Shader::Stage stage)
		{
			std::string stageName;

			switch (stage)
			{
				case Shader::Stage::Vertex:
				{
					stageName = "vertex";
					break;
				}

				case Shader::Stage::TesselationControl:
				{
					stageName = "tesselation control";
					break;
				}

				case Shader::Stage::TesselationEvaluation:
				{
					stageName = "tesselation evaluation";
					break;
				}

				case Shader::Stage::Geometry:
				{
					stageName = "geometry";
					break;
				}

				case Shader::Stage::Fragment:
				{
					stageName = "fragment";
					break;
				}

				case Shader::Stage::Compute:
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

		static shaderc_shader_kind ShaderStageToShaderC(Shader::Stage stage)
		{
			switch (stage)
			{
				case Shader::Stage::Vertex:
				{
					return shaderc_vertex_shader;
				}

				case Shader::Stage::TesselationControl:
				{
					return shaderc_tess_control_shader;
				}

				case Shader::Stage::TesselationEvaluation:
				{
					return shaderc_tess_evaluation_shader;
				}

				case Shader::Stage::Geometry:
				{
					return shaderc_geometry_shader;
				}

				case Shader::Stage::Fragment:
				{
					return shaderc_fragment_shader;
				}

				case Shader::Stage::Compute:
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

		static VkShaderStageFlagBits ShaderStageToVkFlagBit(Shader::Stage stage)
		{
			switch (stage)
			{
				case Shader::Stage::Vertex:
				{
					return VK_SHADER_STAGE_VERTEX_BIT;
				}

				case Shader::Stage::TesselationControl:
				{
					return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				}

				case Shader::Stage::TesselationEvaluation:
				{
					return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				}

				case Shader::Stage::Geometry:
				{
					return VK_SHADER_STAGE_GEOMETRY_BIT;
				}

				case Shader::Stage::Fragment:
				{
					return VK_SHADER_STAGE_FRAGMENT_BIT;
				}

				case Shader::Stage::Compute:
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
		if (specification.StageBitMask & Shader::StageBits::VertexBit)
			loaded &= ReadShaderSource(Shader::Stage::Vertex);

		if (specification.StageBitMask & Shader::StageBits::FragmentBit)
			loaded &= ReadShaderSource(Shader::Stage::Fragment);
	
		if (!loaded)
		{
			std::string errorMessage = "Failed to load source code for shader '{0}'" + specification.Name;
			throw std::runtime_error(errorMessage);
		}

		std::filesystem::path cacheDirectory = VulkanUtils::GetSPIRVCachePath();
		
		Timer shaderCreationTimer;
		{
			CompileOrGetSPIRV(m_GLSLSource, cacheDirectory);
			CreateModules(m_SPIRVBytecode);

			///////////////////////////////////////////////////////////////////////////////////////
			// TODO: move this to its own function, possibly in VulkanPipeline?
			//for (auto& [stage, module] : m_Modules)
			//{
			//	VkPipelineShaderStageCreateInfo shaderStageInfo;
			//	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			//	shaderStageInfo.stage = VulkanUtils::ShaderStageToFlagBit(stage);
			//	shaderStageInfo.module = module;
			//	shaderStageInfo.pName = "main";
			//	shaderStageInfo.pSpecializationInfo = nullptr; // TODO: allows you to specify values for shader constants
			//
			//
			//}
			///////////////////////////////////////////////////////////////////////////////////////
		}
		Z_CORE_TRACE("Shader creation took {0} ms", shaderCreationTimer.ElapsedMillis());

	}

	VulkanShader::~VulkanShader()
	{
		for (auto& [stage, module] : m_Modules)
		{
			vkDestroyShaderModule(VulkanContext::Get()->GetDevice()->Device, module, nullptr);
		}
	}

	bool VulkanShader::ReadShaderSource(Shader::Stage stage)
	{
		bool success = true;

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
			success = false;
			//Z_CORE_ERROR("Failed to read shader source file '{0}'", sourceFilepath.c_str());
		}

		return success;
	}

	void VulkanShader::CompileOrGetSPIRV(const std::unordered_map<Shader::Stage, std::string>& shaderSources, const std::filesystem::path& cacheDirectory)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		const bool optimize = true;
		if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_performance);

		auto& bytecode = m_SPIRVBytecode;
		bytecode.clear();

		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path spirvFilename = m_Specification.Name + "_" + VulkanUtils::ShaderStageToFileExtension(stage) + ".spv";
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
					Z_CORE_ASSERT(false);
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

	void VulkanShader::CreateModules(const std::unordered_map<Shader::Stage, std::vector<uint32_t>>& bytecode)
	{
		for (const auto& [stage, bytes] : bytecode)
		{
			VkShaderModuleCreateInfo moduleInfo{};
			moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleInfo.codeSize = bytes.size() * sizeof(uint32_t);
			moduleInfo.pCode = bytes.data();

			VkDevice device = VulkanContext::GetCurrentDevice()->Device;
			VulkanUtils::ValidateVkResult(vkCreateShaderModule(device, &moduleInfo, nullptr, &m_Modules[stage]));
		}
	}

	void VulkanShader::Bind() const
	{
	}

	void VulkanShader::Unbind() const
	{
	}

	void VulkanShader::SetInt(const std::string& name, int value)
	{
	}

	void VulkanShader::SetIntArray(const std::string& name, uint32_t count, int* values)
	{
	}

	void VulkanShader::SetFloat(const std::string& name, float value)
	{
	}

	void VulkanShader::SetFloat2(const std::string& name, const glm::vec2& values)
	{
	}

	void VulkanShader::SetFloat3(const std::string& name, const glm::vec4& values)
	{
	}

	void VulkanShader::SetFloat4(const std::string& name, const glm::vec4& values)
	{
	}

	void VulkanShader::SetMat2(const std::string& name, const glm::mat2& matrix)
	{
	}

	void VulkanShader::SetMat3(const std::string& name, const glm::mat3& matrix)
	{
	}

	void VulkanShader::SetMat4(const std::string& name, const glm::mat4& matrix)
	{
	}

	std::string VulkanShader::GetSourceFilename(Shader::Stage stage)
	{
		return m_Specification.Name + "." + VulkanUtils::ShaderStageToFileExtension(stage);
	}
}
