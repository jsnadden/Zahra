#pragma once

#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanTexture.h"
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

		virtual ImGuiTextureHandle RegisterTexture(Ref<Texture2D> texture) override;
		virtual void DeregisterTexture(ImGuiTextureHandle textureHandle) override;

		//virtual void SetRenderTarget(Ref<Image2D> renderTarget) override;

		bool OnWindowResizedEvent(WindowResizedEvent& event);

	private:
		Ref<VulkanSwapchain> m_Swapchain;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkImage m_RenderTarget = VK_NULL_HANDLE;
		VkImageView m_LinearisedImageView = VK_NULL_HANDLE;
		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;

		VkExtent2D m_FramebufferSize;

		//Ref<VulkanImage2D> m_RenderTarget;
		//bool m_DefaultRenderTarget = true;

		void CreateDescriptorPool();
		void CreateRenderPass();
		void CreateLinearisedRenderTarget();
		void CreateFramebuffer();

		void Cleanup();


	};

}

