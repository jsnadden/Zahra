#include "zpch.h"
#include "VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanPipeline.h"

namespace Zahra
{

	void VulkanRendererAPI::Init()
	{
		
	}

	void VulkanRendererAPI::Shutdown()
	{
		
	}

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		// TODO: figure out which class "owns" this and forward it this data
	}

	void VulkanRendererAPI::SetClearColour(const glm::vec4& colour)
	{
		// TODO: figure out which class "owns" this and forward it this data
	}

	/////////////////////////////////////////////////////////////////////////////////
	// TEMPORARY
	void VulkanRendererAPI::SetPipeline(const Ref<Pipeline>& pipeline)
	{
		VulkanContext::Get()->GetSwapchain()->SetPipeline(pipeline.As<VulkanPipeline>()->GetVkPipeline());
	}

	void VulkanRendererAPI::Present()
	{

	}
	/////////////////////////////////////////////////////////////////////////////////
}

