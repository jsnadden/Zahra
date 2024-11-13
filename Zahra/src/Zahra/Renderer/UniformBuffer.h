#pragma once

#include "Zahra/Core/Defines.h"

namespace Zahra
{

	class UniformBuffer : public RefCounted
	{
	public:
		virtual ~UniformBuffer() {}

		virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) = 0;

		static Ref<UniformBuffer> Create(uint64_t size);
		static Ref<UniformBuffer> Create(const void* data, uint64_t size, uint64_t offset = 0);
	};

}

