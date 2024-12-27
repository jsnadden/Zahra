#pragma once

#include "Zahra/Core/Ref.h"
#include "Zahra/Renderer/ShaderTypes.h"
#include "Zahra/Renderer/VertexBuffer.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Zahra
{	

	struct ShaderSpecification
	{
		std::string Name;
		std::string SourceSubdirectory = "";
		uint8_t StageBitMask = ShaderStageBits::MinimumGraphicsStageBits;
	};

	class Shader : public RefCounted
	{
	public:

		virtual ~Shader() = default;

		virtual const std::string& GetName() const = 0;
		virtual const VertexBufferLayout& GetVertexLayout() const = 0;

		static Ref<Shader> Create(ShaderSpecification& specification, bool forceCompile = false);

	};


	class ShaderLibrary
	{
	public:
		~ShaderLibrary() { m_Shaders.clear(); }

		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);

		Ref<Shader> Create(ShaderSpecification specification);

		Ref<Shader> Get(const std::string& name);
		bool Exists(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	
	};

}

