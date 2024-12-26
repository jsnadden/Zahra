#pragma once

#include "Zahra/Core/Ref.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/Texture.h"
#include "Zahra/Renderer/UniformBuffer.h"

namespace Zahra
{
	struct ShaderResourceManagerSpecification
	{
		Ref<Shader> Shader;
		uint32_t FirstSet, LastSet; // may require refactoring when moving beyond Vulkan, not sure how DX12 binds uniforms
	};
	
	class ShaderResourceManager : public RefCounted
	{
	public:
		~ShaderResourceManager() {}

		virtual void ProvideResource(const std::string& name, Ref<UniformBufferSet> uniformBufferSet) = 0;
		virtual void ProvideResource(const std::string& name, Ref<Texture2D> texture) = 0;
		virtual void ProvideResource(const std::string& name, const std::vector<Ref<Texture2D>>& textureArray) = 0;

		virtual bool CheckIfComplete() = 0;
		virtual void Update() = 0;

		static Ref<ShaderResourceManager> Create(const ShaderResourceManagerSpecification& specification);
	};

}
