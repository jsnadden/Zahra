#include "zpch.h"
#include "Shader.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Zahra
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Shader Class

	Ref<Shader> Shader::Create(ShaderSpecification& specification)
	{
		Z_CORE_ASSERT(!specification.Name.empty());

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanShader>::Create(specification);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ShaderLibrary Class

    void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
    {
        Z_CORE_ASSERT(!Exists(name), "Shader with this name already exists.");
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Add(const Ref<Shader>& shader)
    {
        auto& name = shader->GetName();
        Add(name, shader);
    }

    Ref<Shader> ShaderLibrary::Get(const std::string& name)
    {
        Z_CORE_ASSERT(Exists(name), "No shader with this name exists.");
        return m_Shaders[name];
    }

    bool ShaderLibrary::Exists(const std::string& name) const
    {
        return m_Shaders.find(name) != m_Shaders.end();
    }

}
