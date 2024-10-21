#pragma once

#include "Zahra/Core/Ref.h"

struct GLFWwindow;

namespace Zahra
{
	class RendererContext : public RefCounted
	{
	public:
		virtual ~RendererContext() = default;
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		static Ref<RendererContext> Create(GLFWwindow* windowHandle);
	};
}
