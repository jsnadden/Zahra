#pragma once

#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanTexture.h"
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

		virtual ImGuiTextureHandle RegisterTexture(Ref<Texture2D> texture) override;
		virtual void DeregisterTexture(ImGuiTextureHandle& handle) override;

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
		void CreateRenderTargetImageView();
		void CreateFramebuffer();

		void Cleanup();


	};

}

