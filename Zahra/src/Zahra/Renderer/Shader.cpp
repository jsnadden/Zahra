#include "zpch.h"
#include "Shader.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Zahra
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Shader Class

    Ref<Shader> Shader::Create(const std::string& filepath)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:    return CreateRef<OpenGLShader>(filepath);
        case RendererAPI::API::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::API::Direct3D is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
    }

    Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:    return CreateRef<OpenGLShader>(name, vertexSource, fragmentSource);
        case RendererAPI::API::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::API::Direct3D is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ShaderLibrary Class

    void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
    {
        Z_PROFILE_FUNCTION();

        Z_CORE_ASSERT(!Exists(name), "Shader with this name already exists.");
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Add(const Ref<Shader>& shader)
    {
        Z_PROFILE_FUNCTION();

        auto& name = shader->GetName();
        Add(name, shader);
    }

    Ref<Shader> ShaderLibrary::Load(const std::string& filepath)
    {
        Z_PROFILE_FUNCTION();

        auto shader = Shader::Create(filepath);
        Add(shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
    {
        Z_PROFILE_FUNCTION();

        auto shader = Shader::Create(filepath);
        Add(name, shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::Get(const std::string& name)
    {
        Z_PROFILE_FUNCTION();

        Z_CORE_ASSERT(Exists(name), "No shader with this name exists.");
        return m_Shaders[name];
    }

    bool ShaderLibrary::Exists(const std::string& name) const
    {
        Z_PROFILE_FUNCTION();

        return m_Shaders.find(name) != m_Shaders.end();
    }

}