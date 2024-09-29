#pragma once

#include "Zahra/Core/Ref.h"
#include "Zahra/Renderer/Pipeline.h"

namespace Zahra
{
	struct CommandBufferSpecification
	{
		Ref<Pipeline> Pipeline;
	};

	class CommandBuffer : public RefCounted
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Record() = 0;

		static Ref<CommandBuffer> Create(const CommandBufferSpecification& specification);
	};
}
