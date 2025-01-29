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
		uint32_t FirstSet = 0, LastSet = 0; // may require refactoring when moving beyond Vulkan, not sure how DX12 binds uniforms
	};
	
	class ShaderResourceManager : public RefCounted
	{
	public:
		~ShaderResourceManager() {}

		// TODO: swap out uniform names for a numerical ID? (Something clever at compile
		// time, or a table in the assets database?)

		// TODO: push constants!!
		
		// For static resources (should only be called prior to being used in rendering)
		virtual void Set(const std::string& name, Ref<UniformBufferPerFrame> uniformBufferPerFrame) = 0;
		virtual void Set(const std::string& name, Ref<Texture2D> texture) = 0;
		virtual void Set(const std::string& name, const std::vector<Ref<Texture2D>>& textureArray) = 0;

		// For dynamic resources (should be called every frame)
		virtual void Update(const std::string& name, Ref<UniformBuffer> uniformBuffer) = 0;
		virtual void Update(const std::string& name, Ref<Texture2D> texture) = 0;
		virtual void Update(const std::string& name, const std::vector<Ref<Texture2D>>& textureArray) = 0;

		//virtual bool AllResourcesValid() = 0;
		virtual void ProcessChanges() = 0;

		static Ref<ShaderResourceManager> Create(const ShaderResourceManagerSpecification& specification);
	};

}
