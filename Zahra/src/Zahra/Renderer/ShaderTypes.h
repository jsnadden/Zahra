#pragma once

namespace Zahra
{
	enum class ShaderDataType
	{
		None = 0,
		Bool,
		Int, Int2, Int3, Int4,
		Float, Float2, Float3, Float4,
		Mat2, Mat3, Mat4
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Bool:   return 1;

		case ShaderDataType::Int:    return 4;
		case ShaderDataType::Int2:   return 4 * 2;
		case ShaderDataType::Int3:   return 4 * 3;
		case ShaderDataType::Int4:   return 4 * 4;

		case ShaderDataType::Float:  return 4;
		case ShaderDataType::Float2: return 4 * 2;
		case ShaderDataType::Float3: return 4 * 3;
		case ShaderDataType::Float4: return 4 * 4;

		case ShaderDataType::Mat2:   return 4 * 4;
		case ShaderDataType::Mat3:   return 4 * 9;
		case ShaderDataType::Mat4:   return 4 * 16;
		}

		Z_CORE_ASSERT(false, "Invalid ShaderDataType");
		return 0;
	}

	enum class ShaderResourceType
	{
		None,
		UniformBuffer,
		UniformBufferSet, // in order to have a unique buffer per frame-in-flight
		StorageBuffer,
		Image,
		Texture2D,
		Texture3D
	};

	enum class ShaderStage
	{
		None,
		Vertex,
		TesselationControl,
		TesselationEvaluation,
		Geometry,
		Fragment,
		Compute
	};

	enum ShaderStageBits
	{
		VertexStageBit					= BIT(0),
		TesselationControlStageBit		= BIT(1),
		TesselationEvaluationStageBit	= BIT(2),
		GeometryStageBit				= BIT(3),
		FragmentStageBit				= BIT(4),

		MinimumGraphicsStageBits = VertexStageBit | FragmentStageBit,
		AllGraphicsStageBits = VertexStageBit | TesselationControlStageBit | TesselationEvaluationStageBit | GeometryStageBit | FragmentStageBit,

		ComputeStageBit					= BIT(5)
	};

	static ShaderStageBits ShaderStageToStageBit(ShaderStage stage)
	{
		switch (stage)
		{
			case ShaderStage::Vertex:
			{
				return ShaderStageBits::VertexStageBit;
			}

			case ShaderStage::TesselationControl:
			{
				return ShaderStageBits::TesselationControlStageBit;
			}

			case ShaderStage::TesselationEvaluation:
			{
				return ShaderStageBits::TesselationEvaluationStageBit;
			}

			case ShaderStage::Geometry:
			{
				return ShaderStageBits::GeometryStageBit;
			}

			case ShaderStage::Fragment:
			{
				return ShaderStageBits::FragmentStageBit;
			}

			case ShaderStage::Compute:
			{
				return ShaderStageBits::ComputeStageBit;
			}

			default:
			{
				Z_CORE_ASSERT(false, "Unrecognised shader stage");
				break;
			}
		}
	}



}
