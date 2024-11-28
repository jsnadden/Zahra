#include "zpch.h"
#include "VulkanRenderPass.h"

#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanShaderUtils.h"
#include "Platform/Vulkan/VulkanUtils.h"

namespace Zahra
{
	namespace VulkanUtils
	{
		VkAttachmentLoadOp VulkanLoadOp(AttachmentLoadOp op)
		{
			switch (op)
			{
				case AttachmentLoadOp::Load:
				{
					return VK_ATTACHMENT_LOAD_OP_LOAD;
					break;
				}
				case AttachmentLoadOp::Clear:
				{
					return VK_ATTACHMENT_LOAD_OP_CLEAR;
					break;
				}
				case AttachmentLoadOp::Unspecified:
				{
					return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					break;
				}
				
			}

			Z_CORE_ASSERT(false, "Unrecognised LoadOp");
			return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
		}

		VkAttachmentStoreOp VulkanStoreOp(AttachmentStoreOp op)
		{
			switch (op)
			{
				case AttachmentStoreOp::Store:
				{
					return VK_ATTACHMENT_STORE_OP_STORE;
					break;
				}
				case AttachmentStoreOp::Unspecified:
				{
					return VK_ATTACHMENT_STORE_OP_DONT_CARE;
					break;
				}

			}

			Z_CORE_ASSERT(false, "Unrecognised StoreOp");
			return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
		}

		VkImageLayout VulkanAttachmentLayout(AttachmentLayout layout)
		{
			switch (layout)
			{
				case AttachmentLayout::Undefined:
				{
					return VK_IMAGE_LAYOUT_UNDEFINED;
					break;
				}
				case AttachmentLayout::Colour:
				{
					return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					break;
				}
				case AttachmentLayout::DepthStencil:
				{
					return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					break;
				}
				case AttachmentLayout::Present:
				{
					return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					break;
				}
			}

			Z_CORE_ASSERT(false, "Unrecognised layout");
			return VK_IMAGE_LAYOUT_MAX_ENUM;
		}
	}

	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specification)
		: m_Specification(specification)
	{
		m_Swapchain = VulkanContext::Get()->GetSwapchain();

		CreateRenderPass();
		CreatePipeline();
		CreateFramebuffers();
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);

		DestroyFramebuffers();
		
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);

		vkDestroyRenderPass(device, m_RenderPass, nullptr);
	}

	void VulkanRenderPass::CreateRenderPass()
	{
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();
		VkFormat depthStencilFormat = m_Swapchain->GetDepthStencilFormat();

		// TODO: multisampling

		VkAttachmentDescription colourAttachment{};
		colourAttachment.format = m_Swapchain->GetSwapchainImageFormat();
		colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colourAttachment.loadOp = VulkanUtils::VulkanLoadOp(m_Specification.LoadOp);
		colourAttachment.storeOp = VulkanUtils::VulkanStoreOp(m_Specification.StoreOp);
		colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colourAttachment.initialLayout = VulkanUtils::VulkanAttachmentLayout(m_Specification.InitialLayout);
		colourAttachment.finalLayout = VulkanUtils::VulkanAttachmentLayout(m_Specification.FinalLayout);

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = depthStencilFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkAttachmentDescription> attachments = { colourAttachment };
		if (m_Specification.HasDepthStencil) attachments.emplace_back(depthAttachment);

		VkAttachmentReference colourAttachmentRef{};
		colourAttachmentRef.attachment = 0;
		colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // data layout the given subpass will treat the image as

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // as opposed to compute
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colourAttachmentRef;
		if (m_Specification.HasDepthStencil) subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VulkanUtils::ValidateVkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass),
			"Main render pass creation failed");
	}

	void VulkanRenderPass::CreatePipeline()
	{
		/////////////////////////////////////////////////////////////////////////////////////////////
		// GATHER DATA
		Ref<VulkanShader> shader = Ref<VulkanShader>(m_Specification.Shader);
		const auto& shaderStageInfos = shader->GetPipelineShaderStageInfos();
		Ref<VulkanSwapchain> swapchain = VulkanContext::Get()->GetSwapchain();
		VkDevice& device = swapchain->GetDevice()->GetVkDevice();
		VkRenderPass renderPass = m_RenderPass;
		VertexBufferLayout vertexLayout = m_Specification.VertexLayout;
		std::vector<VkDescriptorSetLayout> layouts = shader->GetDescriptorSetLayouts();
		// TODO: get push constant ranges

		/////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIGURE DYNAMIC STATE
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		/////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIGURE VERTEX INPUT
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;

		VkVertexInputBindingDescription& vertexInputBinding = vertexInputBindingDescriptions.emplace_back();
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = vertexLayout.GetStride();
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// TODO: add other layouts for instanced transforms etc.

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes(vertexLayout.GetElementCount()); // TODO: add other layout counts here

		uint32_t binding = 0;
		uint32_t location = 0;
		for (const auto& layout : { vertexLayout }) // TODO: any other layouts (e.g. instancing) should be listed here
		{
			for (const auto& element : layout)
			{
				vertexInputAttributes[location].binding = binding;
				vertexInputAttributes[location].location = location;
				vertexInputAttributes[location].format = VulkanUtils::ShaderDataTypeToVulkanFormat(element.Type);
				vertexInputAttributes[location].offset = element.Offset;

				location++;
			}

			binding++;
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1; // TODO: increase to account for additional buffers?
		vertexInputInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributes.size();
		vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

		/////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIGURE PRIMITIVE ASSEMBLY
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // TODO: fill this out based on m_Specification.Topology
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE; // TODO: figure out what this should be set to

		VkPipelineViewportStateCreateInfo viewportStateInfo{};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.scissorCount = 1;
		viewportStateInfo.pViewports = nullptr; // nullptr because these are dynamic states
		viewportStateInfo.pScissors = nullptr;

		/////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIGURE RASTERISATION STAGE
		VkPipelineRasterizationStateCreateInfo rasterizationStateInfo{};
		rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateInfo.depthClampEnable = VK_FALSE;
		rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateInfo.lineWidth = 1.0f;
		rasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateInfo.depthBiasConstantFactor = 0.0f;
		rasterizationStateInfo.depthBiasClamp = 0.0f;
		rasterizationStateInfo.depthBiasSlopeFactor = 0.0f;
		// TODO: figure out where these data come from

		/////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIGURE MULTISAMPLING
		VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
		multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.sampleShadingEnable = VK_FALSE;
		multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleStateInfo.minSampleShading = 1.0f;
		multisampleStateInfo.pSampleMask = nullptr;
		multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleStateInfo.alphaToOneEnable = VK_FALSE;
		// TODO: figure out where these data need to come from

		/////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIGURE DEPTH/STENCIL BUFFER USAGE
		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo{};
		depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateInfo.depthTestEnable = VK_TRUE;
		depthStencilStateInfo.depthWriteEnable = VK_TRUE;
		depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.minDepthBounds = 0.0f;
		depthStencilStateInfo.maxDepthBounds = 1.0f;
		depthStencilStateInfo.stencilTestEnable = VK_FALSE;
		depthStencilStateInfo.front = {};
		depthStencilStateInfo.back = {};

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.logicOp = VK_LOGIC_OP_COPY;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachmentState;
		colorBlendState.blendConstants[0] = 0.0f;
		colorBlendState.blendConstants[1] = 0.0f;
		colorBlendState.blendConstants[2] = 0.0f;
		colorBlendState.blendConstants[3] = 0.0f;

		/////////////////////////////////////////////////////////////////////////////////////////////
		// WITH EVERYTHING IN PLACE, BUILD THE PIPELINE!
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VulkanUtils::ValidateVkResult(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout),
			"Vulkan pipeline layout creation failed");

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());;
		pipelineInfo.pStages = shaderStageInfos.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pRasterizationState = &rasterizationStateInfo;
		pipelineInfo.pMultisampleState = &multisampleStateInfo;
		pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VulkanUtils::ValidateVkResult(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline),
			"Vulkan graphics pipeline creation failed");

		Z_CORE_TRACE("Vulkan graphics pipeline creation successful");

	}

	void VulkanRenderPass::CreateFramebuffers()
	{
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();
		auto& swapchainImageviews = m_Swapchain->GetSwapchainImageViews();
		m_FramebufferSize = m_Swapchain->GetExtent();

		for (auto& view : swapchainImageviews)
		{
			std::vector<VkImageView> attachments = { view };
			if (m_Specification.HasDepthStencil) attachments.emplace_back(m_Swapchain->GetDepthStencilImageView());

			auto& framebuffer = m_Framebuffers.emplace_back();

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = (uint32_t)attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_FramebufferSize.width;
			framebufferInfo.height = m_FramebufferSize.height;
			framebufferInfo.layers = 1;

			VulkanUtils::ValidateVkResult(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer),
				"Vulkan framebuffer creation failed");
		}
	}

	void VulkanRenderPass::DestroyFramebuffers()
	{
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();

		for (auto framebuffer : m_Framebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		m_Framebuffers.clear();
	}

	bool VulkanRenderPass::NeedsResizing()
	{
		VkExtent2D swapchainSize = m_Swapchain->GetExtent();
		return (m_FramebufferSize.width != swapchainSize.width) || (m_FramebufferSize.height != swapchainSize.height);
	}

	void VulkanRenderPass::RefreshFramebuffers()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);

		DestroyFramebuffers();
		CreateFramebuffers();
	}

	const VkFramebuffer& VulkanRenderPass::GetFramebuffer(uint32_t index)
	{
		Z_CORE_ASSERT(index < m_Framebuffers.size());

		return m_Framebuffers[index];
	}

}
