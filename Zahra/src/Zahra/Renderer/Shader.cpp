#include "zpch.h"
#include "Shader.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Zahra
{

    Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:    return new OpenGLShader(vertexSrc, fragmentSrc);
        case RendererAPI::API::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::API::Direct3D is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
    }

}