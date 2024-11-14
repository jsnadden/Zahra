#pragma once

#include "Zahra/Core/Ref.h"
#include "Zahra/Renderer/ShaderTypes.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Zahra
{	

	struct ShaderSpecification
	{
		std::string Name;
		std::filesystem::path SourceDirectory = "Resources/Shaders";
		uint8_t StageBitMask = ShaderStageBits::MinimumGraphicsStageBits;
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

