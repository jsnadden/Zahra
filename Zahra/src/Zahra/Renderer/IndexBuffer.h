#pragma once

#include "VertexBuffer.h"

// TODO: for now we'll use 32-bit indices, but should at some point explore how much
// faster we could get by reducing this to 16-bit
namespace Zahra
{
	class IndexBuffer : public RefCounted
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void SetData(const uint32_t* data, uint64_t size) = 0;

		virtual uint64_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(const uint32_t* indices, uint64_t count);
	};
}
