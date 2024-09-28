#pragma once

#include "Zahra/Core/Ref.h"

namespace Zahra
{

	struct RenderPassSpecification
	{
		int something = 0;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static Ref<RenderPass> Create(RenderPassSpecification specification);
	};

}
