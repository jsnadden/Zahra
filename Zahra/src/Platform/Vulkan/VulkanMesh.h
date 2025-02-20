#pragma once

#include "Platform/Vulkan/VulkanIndexBuffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Zahra/Renderer/Mesh.h"

namespace Zahra
{
	class VulkanStaticMesh : public Mesh
	{
	public:
		VulkanStaticMesh(MeshSpecification specification, const std::filesystem::path& filepath);
		virtual ~VulkanStaticMesh();

		virtual Ref<VertexBuffer> GetVertexBuffer() override { return m_VertexBuffer.As<VertexBuffer>(); }
		virtual Ref<IndexBuffer> GetIndexBuffer() override { return m_IndexBuffer.As<IndexBuffer>(); }

	private:
		MeshSpecification m_Specification;

		Ref<VulkanVertexBuffer> m_VertexBuffer;
		Ref<VulkanIndexBuffer> m_IndexBuffer;

		void LoadFromObj(const std::filesystem::path& filepath);

	};

}
