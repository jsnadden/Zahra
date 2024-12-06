#include "zpch.h"
#include "VulkanRenderPass.h"

#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanShaderUtils.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanUtils.h"

namespace Zahra
{
	namespace VulkanUtils
	{
		static VkAttachmentLoadOp VulkanLoadOp(AttachmentLoadOp op)
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

		static VkAttachmentStoreOp VulkanStoreOp(AttachmentStoreOp op)
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
	}

	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specification)
		: m_Specification(specification)
	{
		m_Swapchain = VulkanContext::Get()->GetSwapchain();

		CreateRenderPass();
		CreatePipeline();
		CreateAttachments();
		CreateFramebuffers();
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);

		DestroyFramebuffers();
		DestroyAttachments();
		
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);

		vkDestroyRenderPass(device, m_RenderPass, nullptr);
	}

	void VulkanRenderPass::CreateRenderPass()
	{
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();
		VkFormat depthStencilFormat = VulkanUtils::GetSupportedDepthStencilFormat();

		// TODO: multisampling

		std::vector<VkAttachmentDescription> attachmentDescriptions;

		VkAttachmentDescription& colourAttachmentDesc = attachmentDescriptions.emplace_back();
		colourAttachmentDesc.format = m_Specification.TargetSwapchain ? m_Swapchain->GetSwapchainImageFormat()
			: VulkanUtils::GetColourFormat(m_Specification.PrimaryAttachment.Format);
		colourAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		colourAttachmentDesc.loadOp = VulkanUtils::VulkanLoadOp(m_Specification.PrimaryAttachment.LoadOp);
		colourAttachmentDesc.storeOp = VulkanUtils::VulkanStoreOp(m_Specification.PrimaryAttachment.StoreOp);
		colourAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// TODO: set these from AttachmentSpecification
		colourAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colourAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// TODO: create additional attachment descriptions

		if (m_Specification.HasDepthStencil)
		{
			VkAttachmentDescription& depthAttachmentDesc = attachmentDescriptions.emplace_back();
			depthAttachmentDesc.format = depthStencilFormat;
			depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkAttachmentReference primaryAttachmentRef{};
		primaryAttachmentRef.attachment = 0;
		primaryAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// TODO: create additional attachment references

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// TODO: multiple subpasses?
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &primaryAttachmentRef; // TODO: additional attachments
		if (m_Specification.HasDepthStencil) subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkPipelineStageFlags stageMask = m_Specification.HasDepthStencil ?
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
			: VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkImageAspectFlags accessMask = m_Specification.HasDepthStencil ?
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			: VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = stageMask;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = stageMask;
		dependency.dstAccessMask = accessMask;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VulkanUtils::ValidateVkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass),
			"Main render pass creation failed");
	}

	void VulkanRenderPass::CreatePipeline()
	{
		Ref<VulkanShader> shader = Ref<VulkanShader>(m_Specification.Shader);
		const auto& shaderStageInfos = shader->GetPipelineShaderStageInfos();
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();
		VertexBufferLayout vertexLayout = shader->GetVertexLayout();
		std::vector<VkDescriptorSetLayout> layouts = shader->GetDescriptorSetLayouts();
		// TODO: get push constant ranges

		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

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

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VulkanUtils::VulkanTopology(m_Specification.Topology);
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE; // TODO: figure out what this should be set to

		VkPipelineViewportStateCreateInfo viewportStateInfo{};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.scissorCount = 1;
		// set to nullptr because these are dynamic states:
		viewportStateInfo.pViewports = nullptr;
		viewportStateInfo.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizationStateInfo{};
		rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateInfo.depthClampEnable = VK_FALSE;
		rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateInfo.lineWidth = 1.0f;
		rasterizationStateInfo.cullMode = m_Specification.BackfaceCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
		rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateInfo.depthBiasConstantFactor = 0.0f;
		rasterizationStateInfo.depthBiasClamp = 0.0f;
		rasterizationStateInfo.depthBiasSlopeFactor = 0.0f;

		// TODO: configure multisampling in specification
		VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
		multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.sampleShadingEnable = VK_FALSE;
		multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleStateInfo.minSampleShading = 1.0f;
		multisampleStateInfo.pSampleMask = nullptr;
		multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleStateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo{};
		depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateInfo.depthTestEnable = m_Specification.HasDepthStencil ? VK_TRUE : VK_FALSE;
		depthStencilStateInfo.depthWriteEnable = m_Specification.HasDepthStencil ? VK_TRUE : VK_FALSE;
		depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.minDepthBounds = 0.0f;
		depthStencilStateInfo.maxDepthBounds = 1.0f;
		depthStencilStateInfo.stencilTestEnable = VK_FALSE; // TODO: stencil?
		depthStencilStateInfo.front = {};
		depthStencilStateInfo.back = {};

		// TODO: configure blending in specification
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		// TODO: configure blending for additional colour attachments

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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0; // TODO: push constants
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
		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VulkanUtils::ValidateVkResult(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline),
			"Vulkan graphics pipeline creation failed");

		Z_CORE_TRACE("Vulkan graphics pipeline creation successful");

	}

	void VulkanRenderPass::CreateAttachments()
	{
		if (m_Specification.TargetSwapchain)
			m_AttachmentSize = m_Swapchain->GetExtent();
		else
			m_AttachmentSize = { m_Specification.AttachmentWidth, m_Specification.AttachmentHeight };

		if (!m_Specification.TargetSwapchain)
		{
			// TODO: set usage flags in AttachmentSpecification?
			m_PrimaryAttachment = Ref<VulkanImage>::Create(m_AttachmentSize.width, m_AttachmentSize.height, m_Specification.PrimaryAttachment.Format, ImageUsage::ColourAttachment);
		}

		// TODO: create additional attachments

		if (m_Specification.HasDepthStencil)
		{
			m_DepthStencilAttachment = Ref<VulkanImage>::Create(m_AttachmentSize.width, m_AttachmentSize.height, ImageFormat::Unspecified, ImageUsage::DepthStencilAttachment);
		}
	}

	void VulkanRenderPass::DestroyAttachments()
	{
		m_PrimaryAttachment.Reset();
		m_DepthStencilAttachment.Reset();

		for (auto& attachment : m_AdditionalAttachments)
			attachment.Reset();
	}

	void VulkanRenderPass::CreateFramebuffers()
	{
		// TODO: if this gets more complicated make a VulkanFramebuffer class to avoid more code duplication

		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();

		if (m_Specification.TargetSwapchain)
		{
			for (auto& view : m_Swapchain->GetSwapchainImageViews())
			{
				std::vector<VkImageView> attachments = { view };

				for (auto& attachment : m_AdditionalAttachments)
					attachments.emplace_back(attachment->GetImageView());

				if (m_Specification.HasDepthStencil)
					attachments.emplace_back(m_DepthStencilAttachment->GetImageView());

				auto& framebuffer = m_Framebuffers.emplace_back();

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = m_RenderPass;
				framebufferInfo.attachmentCount = (uint32_t)attachments.size();
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = m_AttachmentSize.width;
				framebufferInfo.height = m_AttachmentSize.height;
				framebufferInfo.layers = 1;

				VulkanUtils::ValidateVkResult(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer),
					"Vulkan framebuffer creation failed");
			}
		}
		else
		{
			std::vector<VkImageView> attachments = { m_PrimaryAttachment->GetImageView() };

			for (auto& attachment : m_AdditionalAttachments)
				attachments.emplace_back(attachment->GetImageView());

			if (m_Specification.HasDepthStencil)
				attachments.emplace_back(m_DepthStencilAttachment->GetImageView());

			auto& framebuffer = m_Framebuffers.emplace_back();

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = (uint32_t)attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_AttachmentSize.width;
			framebufferInfo.height = m_AttachmentSize.height;
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

	Ref<Texture2D> VulkanRenderPass::TextureFromPrimaryAttachment() const
	{
		Z_CORE_ASSERT(!m_Specification.TargetSwapchain, "This method is not supported for render passes targeting the swapchain images");

		Texture2DSpecification textureSpec{};
		textureSpec.Image = m_PrimaryAttachment.As<Image>();
		// TODO: specify other fields?

		Ref<Texture2D> texture = Ref<VulkanTexture2D>::Create(textureSpec).As<Texture2D>();
		Z_CORE_ASSERT(texture);

		return texture;
	}

	bool VulkanRenderPass::NeedsResizing()
	{
		if (!m_Specification.TargetSwapchain)
			return false;

		VkExtent2D swapchainSize = m_Swapchain->GetExtent();
		return (m_AttachmentSize.width != swapchainSize.width) || (m_AttachmentSize.height != swapchainSize.height);
	}

	void VulkanRenderPass::Refresh()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);

		DestroyFramebuffers();
		DestroyAttachments();
		CreateAttachments();
		CreateFramebuffers();
	}

	const VkFramebuffer& VulkanRenderPass::GetFramebuffer(uint32_t index) const
	{
		if (!m_Specification.TargetSwapchain)
		{
			return m_Framebuffers[0];
		}
		else
		{
			Z_CORE_ASSERT(index < m_Framebuffers.size());
			return m_Framebuffers[index];
		}
	}

}
