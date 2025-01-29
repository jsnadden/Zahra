#include "zpch.h"
#include "VulkanMesh.h"

#include "Zahra/Core/Timer.h"

#ifndef TINYOBJLOADER_IMPLEMENTATION
	#define TINYOBJLOADER_IMPLEMENTATION
#endif

#include "tiny_obj_loader.h"

namespace Zahra
{
	VulkanStaticMesh::VulkanStaticMesh(MeshSpecification specification, const std::filesystem::path& filepath)
		: m_Specification(specification)
	{
		// TODO: make a switch-case and write loaders for other formats
		if (m_Specification.SourceType == MeshFileFormat::obj)
		{
			Timer meshLoadTimer;
			{
				LoadFromObj(filepath);
			}
			Z_CORE_TRACE("Mesh '{0}' took {1} ms to load", m_Specification.Name, meshLoadTimer.ElapsedMillis());
		}
		else
		{
			Z_CORE_ASSERT(false, "Unsupported mesh source file format");
		}
	}

	VulkanStaticMesh::~VulkanStaticMesh()
	{

	}

	void VulkanStaticMesh::LoadFromObj(const std::filesystem::path& filepath)
	{
		std::vector<MeshVertex> vertices;
		std::unordered_map<MeshVertex, uint32_t> vertexToIndexMap; // used to avoid vertex duplication
		std::vector<uint32_t> indices;

		// TODO: replace with a more general mesh/asset loader library
		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> triangles;
		std::vector<tinyobj::material_t> materials;
		std::string warning, error;

		if (!tinyobj::LoadObj(&attributes, &triangles, &materials, &warning, &error, filepath.string().c_str()))
			throw std::runtime_error(warning + error);

		for (auto& triangle : triangles)
		{
			for (auto& index : triangle.mesh.indices)
			{
				MeshVertex vertex;
				
				vertex.Position =
				{
					attributes.vertices[3 * index.vertex_index + 0],
					attributes.vertices[3 * index.vertex_index + 1],
					attributes.vertices[3 * index.vertex_index + 2]
				};

				vertex.Normal =
				{
					attributes.normals[3 * index.normal_index + 0],
					attributes.normals[3 * index.normal_index + 1],
					attributes.normals[3 * index.normal_index + 2]
				};

				if (!attributes.texcoords.empty())
				{
					vertex.TextureCoordinates =
					{
						0.0f + attributes.texcoords[2 * index.texcoord_index + 0],
						1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (vertexToIndexMap.count(vertex) == 0)
				{
					vertexToIndexMap[vertex] = (uint32_t)vertices.size();
					vertices.emplace_back(vertex);
				}

				indices.emplace_back(vertexToIndexMap[vertex]);
			}
		}

		m_VertexBuffer = Ref<VulkanVertexBuffer>::Create((void*)vertices.data(), (uint32_t)(vertices.size() * sizeof(MeshVertex)));
		m_IndexBuffer = Ref<VulkanIndexBuffer>::Create(indices.data(), (uint32_t)indices.size());

	}

}
