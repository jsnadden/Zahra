#pragma once

#include "Zahra/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Zahra
{
	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_windowHandle;

	};
}
