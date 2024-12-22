#include "zpch.h"
#include "Mesh.h"

#include "Platform/Vulkan/VulkanMesh.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{
	Ref<StaticMesh> StaticMesh::Create(MeshSpecification specification)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:  Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanStaticMesh>::Create(specification);
		}

		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}

	const std::string StaticMesh::Filepath(const MeshSpecification& specification)
	{
		std::string filepath = Renderer::GetConfig().MeshSourceDirectory.string();
		std::string filename = specification.Name;

		if (!specification.SourceSubdirectory.empty())
			filepath += "/" + specification.SourceSubdirectory;

		switch (specification.SourceType)
		{
			case MeshFileFormat::fbx:	filename += ".fbx";		break;
			case MeshFileFormat::gltf:	filename += ".gltf";	break;
			case MeshFileFormat::glb:	filename += ".glb";		break;
			case MeshFileFormat::obj:	filename += ".obj";		break;
			case MeshFileFormat::usd:	filename += ".usd";		break;
			case MeshFileFormat::usda:	filename += ".usda";	break;
			case MeshFileFormat::usdc:	filename += ".usdc";	break;
			case MeshFileFormat::usdz:	filename += ".usdz";	break;

			default:
			{
				Z_CORE_ASSERT(false, "Unsupported mesh file format");
				break;
			}
		}

		filepath += "/" + filename;
		return filepath;
	}
}
