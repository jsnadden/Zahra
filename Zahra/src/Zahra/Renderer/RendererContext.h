#pragma once

struct GLFWwindow;

namespace Zahra
{
	class RendererContext
	{
	public:
		virtual ~RendererContext() = default;
		virtual void Init() = 0;
		virtual void Shutdown() = 0;
		virtual void SwapBuffers() = 0;

		static Ref<RendererContext> Create(GLFWwindow* windowHandle);
	};
}
