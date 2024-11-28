#pragma once

#include "Zahra/ImGui/ImGuiLayer.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanImGuiLayer : public ImGuiLayer
	{
	public:
		VulkanImGuiLayer();
		VulkanImGuiLayer(const std::string& name);
		virtual ~VulkanImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& event) override;

		virtual void Begin() override;
		virtual void End() override;

		bool OnWindowResizedEvent(WindowResizedEvent& event);

	private:
		VkDescriptorPool m_DescriptorPool;
		VkRenderPass m_RenderPass;
		std::vector<VkFramebuffer> m_Framebuffers;

	};

}

