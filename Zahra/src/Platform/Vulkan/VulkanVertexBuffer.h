#pragma once

#include "Zahra/Renderer/VertexBuffer.h"

namespace Zahra
{
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		// TODO: there should be an additional argument to choose static/dynamic drawing
		VulkanVertexBuffer(uint32_t size);
		VulkanVertexBuffer(float* vertices, uint32_t size);
		~VulkanVertexBuffer();

		/*virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }
		virtual const VertexBufferLayout& GetLayout() const override { return m_Layout; }*/

		virtual void SetData(const void* data, uint32_t size) override;

	private:
		//VertexBufferLayout m_Layout;

	};
}
