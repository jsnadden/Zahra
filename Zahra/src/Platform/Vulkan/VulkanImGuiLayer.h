#pragma once

#include "Zahra/ImGui/ImGuiLayer.h"

#include <vulkan/vulkan.h>

namespace Zahra
{

	// TODO: it may be worth rewriting some of the imgui_implvulkan stuff to better gel
	// with my own renderer. For example, I'd get to choose which colour space it uses!
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

		virtual ImGuiResourceHandle RegisterTexture(Ref<Texture2D> texture) override;
		virtual void DeregisterTexture(ImGuiResourceHandle textureHandle) override;

	private:
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_Framebuffers;
		VkExtent2D m_FramebufferSize;

		void CreateDescriptorPool();
		void CreateRenderPass();
		void CreateFramebuffers();
		void DestroyFramebuffers();

	};

}

