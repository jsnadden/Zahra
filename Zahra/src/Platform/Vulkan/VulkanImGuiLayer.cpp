#include "zpch.h"
#include "VulkanImGuiLayer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Zahra/Core/Application.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_Vulkan.h>
#include <GLFW/glfw3.h>
#include <ImGuizmo.h>
#include <vulkan/vk_enum_string_helper.h>

namespace Zahra
{
	VulkanImGuiLayer::VulkanImGuiLayer()
	{}

	VulkanImGuiLayer::VulkanImGuiLayer(const std::string& name)
	{
		m_DebugName = name;
	}

	VulkanImGuiLayer::~VulkanImGuiLayer()
	{}

	void VulkanImGuiLayer::OnAttach()
	{
		Window& window = Application::Get().GetWindow();
		GLFWwindow* windowHandle = static_cast<GLFWwindow*>(window.GetWindowHandle());
		Ref<VulkanContext> context = window.GetRendererContext().As<VulkanContext>();
		Ref<VulkanDevice> device = context->GetDevice();

		m_Swapchain = context->GetSwapchain();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		io.IniFilename = "./Config/imgui.ini";

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetColourTheme();		

		ImGui_ImplGlfw_InitForVulkan(windowHandle, true);		

		CreateDescriptorPool();
		CreateDefaultRenderTarget();
		CreateRenderPass();
		CreateFramebuffer();

		ImGui_ImplVulkan_InitInfo imguiInfo = {};
		imguiInfo.Instance = context->GetVulkanInstance();
		imguiInfo.PhysicalDevice = device->GetPhysicalDevice();
		imguiInfo.Device = device->GetVkDevice();
		imguiInfo.QueueFamily = device->GetQueueFamilyIndices().GraphicsIndex.value();
		imguiInfo.Queue = device->GetGraphicsQueue();
		imguiInfo.PipelineCache = VK_NULL_HANDLE; // TODO: not sure if we'll use a pipeline cache, but might have to return here
		imguiInfo.RenderPass = m_RenderPass;
		imguiInfo.Subpass = 0;
		imguiInfo.DescriptorPool = m_DescriptorPool;
		imguiInfo.MinImageCount = 2;
		imguiInfo.ImageCount = Application::Get().GetSpecification().RendererConfig.FramesInFlight;
		imguiInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		imguiInfo.Allocator = nullptr; // TODO: VMA allocations
		imguiInfo.CheckVkResultFn = [](VkResult result)
			{
				std::string errorMsg = "ImGui's Vulkan implementation has encountered unsuccessful VkResult value ";
				errorMsg += string_VkResult(result);
				Z_CORE_ASSERT(result == VK_SUCCESS, errorMsg);
			};
		
		ImGui_ImplVulkan_Init(&imguiInfo);

		// TODO: write a font library so that I don't have to rely on imgui's internal font vector
		io.FontDefault = io.Fonts->AddFontFromFileTTF("..\\Meadow\\Resources\\Fonts\\Inter\\Inter-Regular.ttf", 18.0f); // font 0
		io.Fonts->AddFontFromFileTTF("..\\Meadow\\Resources\\Fonts\\Inter\\Inter-Bold.ttf", 18.0f); // font 1

	}

	void VulkanImGuiLayer::OnDetach()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		DestroyFramebufferAndRenderPass();
		m_RenderTarget.Reset();
		vkDestroyDescriptorPool(m_Swapchain->GetVkDevice(), m_DescriptorPool, nullptr);

	}

	void VulkanImGuiLayer::OnEvent(Event& event)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			event.Handled |= event.IsInCategory(EventCategoryMouse) && io.WantCaptureMouse;
			event.Handled |= event.IsInCategory(EventCategoryKeyboard) && io.WantCaptureKeyboard;
		}

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizedEvent>(Z_BIND_EVENT_FN(VulkanImGuiLayer::OnWindowResizedEvent));
	}

	void VulkanImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void VulkanImGuiLayer::End()
	{
		/*if (m_Swapchain->Invalidated())
		{
			const auto& device = m_Swapchain->GetVkDevice();

			vkDeviceWaitIdle(device);
			vkDestroyFramebuffer(device, m_Framebuffer, nullptr);

			if (m_DefaultRenderTarget)
			{
				uint32_t width = m_Swapchain->GetWidth();
				uint32_t height = m_Swapchain->GetHeight();
				m_RenderTarget->Resize(width, height);
			}

			CreateFramebuffer();
		}*/

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		ImGui::Render();
		auto drawData = ImGui::GetDrawData();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		VkClearValue clearColour = {{ 0.0f, 0.0f, 0.0f }};

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_RenderPass;
		renderPassBeginInfo.framebuffer = m_Framebuffer;
		renderPassBeginInfo.renderArea.extent = m_RenderTarget->GetDimensions();
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColour;
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

		vkCmdEndRenderPass(commandBuffer);
	}

	ImGuiTextureHandle VulkanImGuiLayer::RegisterTexture(Ref<Texture2D> texture)
	{
		VkDescriptorImageInfo imageInfo = texture.As<VulkanTexture2D>()->GetVkDescriptorImageInfo();
		VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(imageInfo.sampler,
			imageInfo.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		
#if 0
		VkDescriptorSetLayout layout;
		VkDescriptorSet descriptorSet;

		VkDescriptorSetLayoutBinding binding{};
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.binding = 0;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &binding;
		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout);

		VkDescriptorSetAllocateInfo descriptorSetAllocationInfo{};
		descriptorSetAllocationInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocationInfo.descriptorPool = m_DescriptorPool;
		descriptorSetAllocationInfo.descriptorSetCount = 1;
		descriptorSetAllocationInfo.pSetLayouts = &layout;
		vkAllocateDescriptorSets(device, &descriptorSetAllocationInfo, &descriptorSet);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &texture.As<VulkanTexture2D>()->GetVkDescriptorImageInfo();
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
#endif

		return (void*)descriptorSet;
	}

	void VulkanImGuiLayer::DeregisterTexture(ImGuiTextureHandle textureHandle)
	{
		const VkDescriptorSet descriptorSet = (VkDescriptorSet)textureHandle;
		vkFreeDescriptorSets(VulkanContext::GetCurrentVkDevice(), m_DescriptorPool, 1, &descriptorSet);
	}

	void VulkanImGuiLayer::SetRenderTarget(Ref<Image2D> renderTarget)
	{
		DestroyFramebufferAndRenderPass();

		m_RenderTarget = renderTarget.As<VulkanImage2D>();
		m_DefaultRenderTarget = false;

		CreateRenderPass();
		CreateFramebuffer();
	}

	bool VulkanImGuiLayer::OnWindowResizedEvent(WindowResizedEvent& event)
	{
		if (event.GetWidth() == 0 || event.GetHeight() == 0)
			return false;

		const auto& device = m_Swapchain->GetVkDevice();

		vkDeviceWaitIdle(device);
		vkDestroyFramebuffer(device, m_Framebuffer, nullptr);

		if (m_DefaultRenderTarget)
		{
			uint32_t width = event.GetWidth();
			uint32_t height = event.GetHeight();
			m_RenderTarget->Resize(width, height);
		}

		CreateFramebuffer();
		
		return false;
	}

	void VulkanImGuiLayer::CreateDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }
		}; // TODO: figure out how many descriptor sets I'll actually need

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10; // TODO: figure out how many descriptor sets I'll actually need in TOTAL
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();

		// TODO: VMA allocations
		VulkanUtils::ValidateVkResult(vkCreateDescriptorPool(VulkanContext::GetCurrentVkDevice(), &poolInfo, nullptr, &m_DescriptorPool),
			"Vulkan descriptor pool creation failed");
	}

	void VulkanImGuiLayer::CreateRenderPass()
	{
		// TODO: multisampling?

		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.flags = 0;
		attachmentDescription.format = VulkanUtils::VulkanFormat(ImageFormat::SRGBA);
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = m_DefaultRenderTarget ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = m_DefaultRenderTarget ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colourAttachmentRef{};
		colourAttachmentRef.attachment = 0;
		colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colourAttachmentRef;

		std::vector<VkSubpassDependency> subpassDependencies;
		{
			auto& previousPass = subpassDependencies.emplace_back();
			previousPass.srcSubpass = VK_SUBPASS_EXTERNAL;
			previousPass.dstSubpass = 0;
			previousPass.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			previousPass.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			previousPass.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			previousPass.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			previousPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			auto& nextPass = subpassDependencies.emplace_back();
			nextPass.srcSubpass = 0;
			nextPass.dstSubpass = VK_SUBPASS_EXTERNAL;
			nextPass.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			nextPass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			nextPass.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			nextPass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			nextPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attachmentDescription;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = subpassDependencies.size();
		renderPassInfo.pDependencies = subpassDependencies.data();

		VulkanUtils::ValidateVkResult(vkCreateRenderPass(m_Swapchain->GetVkDevice(), &renderPassInfo, nullptr, &m_RenderPass),
			"Main render pass creation failed");
	}

	void VulkanImGuiLayer::CreateDefaultRenderTarget()
	{
		ImageSpecification imageSpec{};
		imageSpec.Format = ImageFormat::SRGBA;
		imageSpec.Width = m_Swapchain->GetWidth();
		imageSpec.Height = m_Swapchain->GetHeight();
		imageSpec.Sampled = true;

		m_RenderTarget = Ref<VulkanImage2D>::Create(imageSpec);
	}

	void VulkanImGuiLayer::CreateFramebuffer()
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &m_RenderTarget->GetVkImageView();
		framebufferInfo.width = m_RenderTarget->GetWidth();
		framebufferInfo.height = m_RenderTarget->GetHeight();
		framebufferInfo.layers = 1;

		VulkanUtils::ValidateVkResult(vkCreateFramebuffer(m_Swapchain->GetVkDevice(), &framebufferInfo, nullptr, &m_Framebuffer),
			"Vulkan framebuffer creation failed");
	}

	void VulkanImGuiLayer::DestroyFramebufferAndRenderPass()
	{
		const auto& device = m_Swapchain->GetVkDevice();
		vkDeviceWaitIdle(device);
		vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
		vkDestroyRenderPass(device, m_RenderPass, nullptr);
	}

}

	

