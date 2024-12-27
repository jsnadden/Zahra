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
		static std::string ShaderStageToFileExtension(ShaderStage stage)
		{
			std::string extension;

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

				default: Z_CORE_ASSERT(false, "Unrecognised shader stage");
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

				default: Z_CORE_ASSERT(false, "Unrecognised shader stage");
			}

			return stageName;
		}

		static shaderc_shader_kind ShaderStageToShaderC(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:					return shaderc_vertex_shader;
				case ShaderStage::TesselationControl:		return shaderc_tess_control_shader;
				case ShaderStage::TesselationEvaluation:	return shaderc_tess_evaluation_shader;
				case ShaderStage::Geometry:					return shaderc_geometry_shader;
				case ShaderStage::Fragment:					return shaderc_fragment_shader;
				case ShaderStage::Compute:					return shaderc_compute_shader;			}

			Z_CORE_ASSERT(false, "Unrecognised shader stage");
			return shaderc_vertex_shader;
		}

		static ShaderDataType SPIRTypeToShaderDataType(spirv_cross::SPIRType type)
		{
			// TODO: currently only works for non-struct attributes of limited types
			if (type.columns == 1)
			{
				switch (type.vecsize)
				{
					case 1: // scalar attribute
					{
						switch (type.basetype)
						{
							case spirv_cross::SPIRType::BaseType::Boolean:	return ShaderDataType::Bool;
							case spirv_cross::SPIRType::BaseType::Int:		return ShaderDataType::Int;
							case spirv_cross::SPIRType::BaseType::Float:	return ShaderDataType::Float;

							default:
							{
								Z_CORE_ASSERT(false, "Unrecognised variable type from shader reflection");
								break;
							}
						}
						break;
					}
					case 2:
					{
						switch (type.basetype)
						{
							case spirv_cross::SPIRType::BaseType::Int:		return ShaderDataType::Int2;
							case spirv_cross::SPIRType::BaseType::Float:	return ShaderDataType::Float2;

							default:
							{
								Z_CORE_ASSERT(false, "Unrecognised variable type from shader reflection");
								break;
							}
						}
						break;
					}
					case 3:
					{
						switch (type.basetype)
						{
							case spirv_cross::SPIRType::BaseType::Int:		return ShaderDataType::Int3;
							case spirv_cross::SPIRType::BaseType::Float:	return ShaderDataType::Float3;

							default:
							{
								Z_CORE_ASSERT(false, "Unrecognised variable type from shader reflection");
								break;
							}
						}
						break;
					}
					case 4:
					{
						switch (type.basetype)
						{
							case spirv_cross::SPIRType::BaseType::Int:		return ShaderDataType::Int4;
							case spirv_cross::SPIRType::BaseType::Float:	return ShaderDataType::Float4;

							default:
							{
								Z_CORE_ASSERT(false, "Unrecognised variable type from shader reflection");
								break;
							}
						}
					}

					default:
					{
						Z_CORE_ASSERT(false, "Unrecognised variable type from shader reflection");
						break;
					}
				}
			}
			else
			{
				switch (type.columns)
				{
					case 2:	return ShaderDataType::Mat2;
					case 3:	return ShaderDataType::Mat3;
					case 4:	return ShaderDataType::Mat4;

					default:
					{
						Z_CORE_ASSERT(false, "Unrecognised variable type from shader reflection");
						break;
					}
				}
			}
			
			Z_CORE_ASSERT(false, "Unrecognised variable type from shader reflection");
			return ShaderDataType::None;
		}

		static VkShaderStageFlagBits ShaderStageToVkFlagBit(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:					return VK_SHADER_STAGE_VERTEX_BIT;
				case ShaderStage::TesselationControl:		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				case ShaderStage::TesselationEvaluation:	return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				case ShaderStage::Geometry:					return VK_SHADER_STAGE_GEOMETRY_BIT;
				case ShaderStage::Fragment:					return VK_SHADER_STAGE_FRAGMENT_BIT;
				case ShaderStage::Compute:					return VK_SHADER_STAGE_COMPUTE_BIT;
			}

			Z_CORE_ASSERT(false, "Unrecognised shader stage");
			return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		}

		static VkDescriptorType ShaderResourceTypeToVkDescriptorType(ShaderResourceType type)
		{
			switch (type)
			{
				case ShaderResourceType::UniformBuffer:		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				case ShaderResourceType::UniformBufferSet:	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;					
				case ShaderResourceType::StorageBuffer:		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;					
				case ShaderResourceType::Texture2D:			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;					
			}

			Z_CORE_ASSERT(false, "Unknown or unsupported ShaderResourceType");
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
