#include "zpch.h"
#include "VulkanImGuiLayer.h"

#include "Platform/Vulkan/VulkanContext.h"
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
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		io.IniFilename = "./Config/imgui.ini";

		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetColourTheme();

		Window& window = Application::Get().GetWindow();

		GLFWwindow* windowHandle = static_cast<GLFWwindow*>(window.GetWindowHandle());
		ImGui_ImplGlfw_InitForVulkan(windowHandle, true);

		Ref<VulkanContext> context = window.GetRendererContext().As<VulkanContext>();
		Ref<VulkanSwapchain> swapchain = context->GetSwapchain();
		Ref<VulkanDevice> device = context->GetDevice();


		//////////////////////////////////////////////////////////////////////////////////////
		// INITIALISE VULKAN IMGUI IMPLEMENTATION
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		// TODO: test to see how much of this overkill allocation I actually need

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1;
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();

		// TODO: VMA allocations
		VulkanUtils::ValidateVkResult(vkCreateDescriptorPool(device->GetVkDevice(), &poolInfo, nullptr, &m_DescriptorPool),
			"Vulkan descriptor pool creation failed");

		/*VkAttachmentDescription attachment = {};
		attachment.format = swapchain->GetImageFormat();
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colourAttachment = {};
		colourAttachment.attachment = 0;
		colourAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colourAttachment;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		VulkanUtils::ValidateVkResult(vkCreateRenderPass(device->GetVkDevice(), &renderPassInfo, nullptr, &m_RenderPass),
			"ImGui render pass creation failed");*/

		ImGui_ImplVulkan_InitInfo imguiInfo = {};
		imguiInfo.Instance = context->GetVulkanInstance();
		imguiInfo.PhysicalDevice = device->GetPhysicalDevice();
		imguiInfo.Device = device->GetVkDevice();
		imguiInfo.QueueFamily = device->GetQueueFamilyIndices().GraphicsIndex.value();
		imguiInfo.Queue = device->GetGraphicsQueue();
		imguiInfo.PipelineCache = VK_NULL_HANDLE; // TODO: not sure if we'll use a pipeline cache, but might have to return here
		//imguiInfo.RenderPass = swapchain->GetVkRenderPass();
		imguiInfo.Subpass = 0;
		imguiInfo.DescriptorPool = m_DescriptorPool;
		imguiInfo.MinImageCount = swapchain->GetImageCount();
		imguiInfo.ImageCount = swapchain->GetImageCount();
		imguiInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		imguiInfo.Allocator = nullptr; // TODO: VMA allocations
		imguiInfo.CheckVkResultFn = [](VkResult result) { return VulkanUtils::ValidateVkResult(result, "Unsuccessful VkResult within ImGui"); };
		
		ImGui_ImplVulkan_Init(&imguiInfo);

		// TODO: write a font library so that I don't have to rely on imgui's internal font vector
		io.FontDefault = io.Fonts->AddFontFromFileTTF("..\\Meadow\\Resources\\Fonts\\Inter\\Inter-Regular.ttf", 18.0f); // font 0
		io.Fonts->AddFontFromFileTTF("..\\Meadow\\Resources\\Fonts\\Inter\\Inter-Bold.ttf", 18.0f); // font 1

		//////////////////////////////////////////////////////////////////////////////////////
		// CREATE IMGUI FRAMEBUFFERS
		/*uint32_t framesInFlight = swapchain->GetFramesInFlight();
		m_Framebuffers.resize(framesInFlight);
		VkImageView???
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width = swapchain->GetExtent().width;
		framebufferInfo.height = swapchain->GetExtent().height;
		framebufferInfo.layers = 1;

		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			VkImageView imageView???
			framebufferInfo.pAttachments = &[i];???
			VulkanUtils::ValidateVkResult(vkCreateFramebuffer(device->GetVkDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
		}*/

	}

	void VulkanImGuiLayer::OnDetach()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		/*for (auto& fb : m_Framebuffers)
		{
			vkDestroyFramebuffer(device, fb, nullptr);
		}
		m_Framebuffers.clear();

		vkDestroyRenderPass(device, m_RenderPass, nullptr);*/
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

		// TODO: need to set things up so that the main app renderer targets a non-swapchain
		// framebuffer, which is ultimately passed into here for ImGui to draw on top of. The
		// following should be modified to receive that (as well as the VulkanRendererAPI code):

		/*VkClearValue clearColour = { { 0.01f, 0.02f, 0.01f, 1.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_RenderPass;
		renderPassBeginInfo.framebuffer = m_Framebuffers[frameIndex];
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColour;
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);*/

		//ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

		//vkCmdEndRenderPass(commandBuffer);
	}

	bool VulkanImGuiLayer::OnWindowResizedEvent(WindowResizedEvent& event)
	{
		// TODO: come back to this if imgui has difficulties with swapchain recreation...
		/*if (event.GetWidth() > 0 || event.GetHeight() > 0)
		{
			
		}*/

		return false;
	}

}

