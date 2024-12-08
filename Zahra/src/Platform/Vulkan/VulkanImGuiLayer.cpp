#include "zpch.h"
#include "VulkanImGuiLayer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Zahra/Core/Application.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_Vulkan.h>
#include <GLFW/glfw3.h>
#include <ImGuizmo.h>

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
		Ref<VulkanSwapchain> swapchain = context->GetSwapchain();
		Ref<VulkanDevice> device = context->GetDevice();

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
		CreateRenderPass();
		CreateFramebuffers();

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
		imguiInfo.ImageCount = m_Framebuffers.size();
		imguiInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		imguiInfo.Allocator = nullptr; // TODO: VMA allocations
		imguiInfo.CheckVkResultFn = [](VkResult result) { Z_CORE_ASSERT(result == VK_SUCCESS, "Unsuccessful VkResult within ImGui"); };
		
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

		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);

		DestroyFramebuffers();
		vkDestroyRenderPass(device, m_RenderPass, nullptr);
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	}

	void VulkanImGuiLayer::OnEvent(Event& event)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			event.Handled |= event.IsInCategory(EventCategoryMouse) && io.WantCaptureMouse;
			event.Handled |= event.IsInCategory(EventCategoryKeyboard) && io.WantCaptureKeyboard;
		}

	}

	void VulkanImGuiLayer::Begin()
	{
		if (VulkanContext::Get()->GetSwapchain()->Invalidated())
		{
			DestroyFramebuffers();
			CreateFramebuffers();
		}

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void VulkanImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		Ref<VulkanSwapchain> swapchain = VulkanContext::Get()->GetSwapchain();
		VkCommandBuffer commandBuffer = swapchain->GetCurrentDrawCommandBuffer();

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
		renderPassBeginInfo.framebuffer = m_Framebuffers[swapchain->GetImageIndex()];
		renderPassBeginInfo.renderArea.extent = swapchain->GetExtent();
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColour;
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

		vkCmdEndRenderPass(commandBuffer);
	}

	void* VulkanImGuiLayer::RegisterTexture(Ref<Texture2D> texture)
	{
		VkDevice device = VulkanContext::GetCurrentVkDevice();
		
		//void* imageHandle = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		
		VkDescriptorSetLayout layout;
		VkDescriptorSet descriptorSet;

		VkDescriptorSetLayoutBinding binding{};
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
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

		return (void*)descriptorSet;
	}

	void VulkanImGuiLayer::CreateDescriptorPool()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER,					1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,				1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,				1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,		1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,		1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,			1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,	1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,			1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1;
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();

		// TODO: VMA allocations
		VulkanUtils::ValidateVkResult(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool),
			"Vulkan descriptor pool creation failed");
	}

	void VulkanImGuiLayer::CreateRenderPass()
	{
		Ref<VulkanSwapchain> swapchain = VulkanContext::Get()->GetSwapchain();
		VkDevice& device = swapchain->GetDevice()->GetVkDevice();

		// TODO: multisampling?

		VkAttachmentDescription colourAttachment{};
		colourAttachment.format = swapchain->GetSwapchainImageFormat();
		colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colourAttachment.loadOp = m_ClearSwapchain ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colourAttachment.initialLayout = m_ClearSwapchain ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colourAttachmentRef{};
		colourAttachmentRef.attachment = 0;
		colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // data layout the given subpass will treat the image as

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // as opposed to compute
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colourAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colourAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VulkanUtils::ValidateVkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass),
			"Main render pass creation failed");
	}

	void VulkanImGuiLayer::CreateFramebuffers()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		Ref<VulkanSwapchain> swapchain = VulkanContext::Get()->GetSwapchain();
		auto& swapchainImageviews = swapchain->GetSwapchainImageViews();
		m_FramebufferSize = swapchain->GetExtent();

		for (auto& view : swapchainImageviews)
		{
			auto& framebuffer = m_Framebuffers.emplace_back();

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &view;
			framebufferInfo.width = m_FramebufferSize.width;
			framebufferInfo.height = m_FramebufferSize.height;
			framebufferInfo.layers = 1;

			VulkanUtils::ValidateVkResult(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer),
				"Vulkan framebuffer creation failed");
		}
	}

	void VulkanImGuiLayer::DestroyFramebuffers()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		for (auto framebuffer : m_Framebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		m_Framebuffers.clear();
	}


}

	

