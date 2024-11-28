#pragma once

#include "Zahra/Renderer/ShaderTypes.h"

#include <filesystem>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <vulkan/vulkan.h> 

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

		static VkDescriptorType ShaderResourceTypeToVkDescriptorType(ShaderResourceType type)
		{
			switch (type)
			{
			case ShaderResourceType::UniformBuffer:
			case ShaderResourceType::UniformBufferSet:
			{
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			}
			case ShaderResourceType::StorageBuffer:
			{
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			}
			case ShaderResourceType::Texture2D:
			{
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			}

			default: Z_CORE_ASSERT(false, "Unknown or unsupported ShaderResourceType"); break;
			}

			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}

		static VkFormat ShaderDataTypeToVulkanFormat(ShaderDataType type)
		{
			switch (type)
			{
			case ShaderDataType::Int:       return VK_FORMAT_R32_SINT;
			case ShaderDataType::Int2:      return VK_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:      return VK_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:      return VK_FORMAT_R32G32B32A32_SINT;

			case ShaderDataType::Float:     return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2:    return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
			Z_CORE_ASSERT(false);
			return VK_FORMAT_UNDEFINED;
		}

	}
}
