#pragma once

#include "Zahra/Core/Ref.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Zahra
{
	enum class ShaderStage
	{
		Vertex,
		TesselationControl,
		TesselationEvaluation,
		Geometry,
		Fragment,
		Compute
	};

	enum ShaderStageBits
	{
		VertexBit = 0x00000001,
		TesselationControlBit = 0x00000002,
		TesselationEvaluationBit = 0x00000004,
		GeometryBit = 0x00000008,
		FragmentBit = 0x00000010,
		ComputeBit = 0x00000020,
		AllGraphics = 0x0000001f
	};

	struct ShaderSpecification
	{
		std::string Name;
		std::filesystem::path SourceDirectory = "Resources/Shaders";
		uint32_t StageBitMask = ShaderStageBits::AllGraphics;
	};

	class Shader : public RefCounted
	{
	public:

		virtual ~Shader() = default;

		// TODO: remove these - Vulkan pipelines are immutable, and all bindings are configured in advance!!
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create(ShaderSpecification& specification);

		/*virtual void SetInt(const  std::string& name, int value) = 0;
		virtual void SetIntArray(const  std::string& name, uint32_t count, int* values) = 0;

		virtual void SetFloat(const  std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& values) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec4& values) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& values) = 0;

		virtual void SetMat2(const std::string& name, const glm::mat2& matrix) = 0;
		virtual void SetMat3(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& matrix) = 0;*/

	};


	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);

		//Ref<Shader> Load(const std::string& filepath);
		//Ref<Shader> Load(const std::string& name, const std::string& filepath);

		Ref<Shader> Get(const std::string& name);
		bool Exists(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	
	};

}

