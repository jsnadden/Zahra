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

		virtual void* RegisterTexture(Ref<Texture2D> texture) override;

	private:
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		void CreateDescriptorPool();

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		void CreateRenderPass();

		std::vector<VkFramebuffer> m_Framebuffers;
		VkExtent2D m_FramebufferSize;
		void CreateFramebuffers();
		void DestroyFramebuffers();

	};

}

