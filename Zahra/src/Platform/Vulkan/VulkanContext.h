#pragma once

#include "Zahra/Renderer/GraphicsContext.h"

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Zahra
{
	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* handle);

		virtual void Init() override;
		virtual void Shutdown() override;
		virtual void SwapBuffers() override;

	private:
		
		VkInstance m_VulkanInstance;

	};
}
