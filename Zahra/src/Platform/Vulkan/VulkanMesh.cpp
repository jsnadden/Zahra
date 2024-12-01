#include "zpch.h"
#include "VulkanMesh.h"

#include "Zahra/Core/Timer.h"

#ifndef TINYOBJLOADER_IMPLEMENTATION
	#define TINYOBJLOADER_IMPLEMENTATION
#endif

#include "tiny_obj_loader.h"

namespace Zahra
{
	VulkanStaticMesh::VulkanStaticMesh(MeshSpecification specification)
		: m_Specification(specification)
	{
		if (strcmp(specification.Filepath.extension().string().c_str(), ".obj") == 0)
		{
			Timer meshLoadTimer;
			{
				LoadFromObj();
			}
			Z_CORE_TRACE("Loading mesh took {0} ms", meshLoadTimer.ElapsedMillis());
		}
	}

	VulkanStaticMesh::~VulkanStaticMesh()
	{

	}

	void VulkanStaticMesh::LoadFromObj()
	{
		std::vector<MeshVertex> vertices;
		std::unordered_map<MeshVertex, uint32_t> vertexToIndexMap; // used to avoid vertex duplication
		std::vector<uint32_t> indices;

		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> triangles;
		std::vector<tinyobj::material_t> materials;
		std::string warning, error;

		if (!tinyobj::LoadObj(&attributes, &triangles, &materials, &warning, &error, m_Specification.Filepath.string().c_str())) {
			throw std::runtime_error(warning + error);
		}

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

				if (!attributes.texcoords.empty())
				{
					vertex.TextureCoordinates =
					{
						attributes.texcoords[2 * index.texcoord_index + 0],
						1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (vertexToIndexMap.count(vertex) == 0)
				{
					vertexToIndexMap[vertex] = vertices.size();
					vertices.emplace_back(vertex);
				}

				indices.emplace_back(vertexToIndexMap[vertex]);
			}
		}

		m_VertexBuffer = Ref<VulkanVertexBuffer>::Create((void*)vertices.data(), vertices.size() * sizeof(MeshVertex));
		m_IndexBuffer = Ref<VulkanIndexBuffer>::Create(indices.data(), indices.size());

	}

}
