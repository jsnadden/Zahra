#pragma once

#include "Zahra/Core/Defines.h"

namespace Zahra
{

	class UniformBuffer : public RefCounted
	{
	public:
		virtual ~UniformBuffer() {}

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		static Ref<UniformBuffer> Create(uint32_t size);
		static Ref<UniformBuffer> Create(const void* data, uint32_t size, uint32_t offset = 0);
	};

	class UniformBufferSet : public RefCounted
	{
	public:
		virtual ~UniformBufferSet() {}

		virtual Ref<UniformBuffer> Get() = 0;
		virtual Ref<UniformBuffer> Get(uint32_t frame) = 0;
		virtual void Set(uint32_t frame, Ref<UniformBuffer> buffer) = 0;
		virtual void SetData(uint32_t frame, const void* data, uint32_t size, uint32_t offset = 0) = 0;

		static Ref<UniformBufferSet> Create(uint32_t bufferSize, uint32_t framesInFlight = 0);
	};

}

