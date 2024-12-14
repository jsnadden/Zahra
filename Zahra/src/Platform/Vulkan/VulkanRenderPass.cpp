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
		ValidateSpecification();

		m_Swapchain = VulkanContext::Get()->GetSwapchain();

		CreateRenderPass();
		CreatePipeline();
		CreateFramebuffer();
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);

		DestroyFramebuffer();
		
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);

		vkDestroyRenderPass(device, m_RenderPass, nullptr);
	}

	const Ref<Framebuffer> VulkanRenderPass::GetFramebuffer() const
	{
		return m_Framebuffer;
	}

	void VulkanRenderPass::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.FramebufferSpec.Width = width;
		m_Specification.FramebufferSpec.Height = height;
		DestroyFramebuffer();
		CreateFramebuffer();
	}

	const VkFramebuffer& VulkanRenderPass::GetVkFramebuffer()
	{
		return m_VkFramebuffers[m_Swapchain->GetImageIndex()];
	}

	std::vector<VkClearValue> const VulkanRenderPass::GetClearValues()
	{
		return m_Framebuffer->GetClearValues();
	}

	void VulkanRenderPass::CreateRenderPass()
	{
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();
		bool hasDepthStencil = m_Specification.FramebufferSpec.HasDepthStencil;
		VkFormat depthStencilFormat = VulkanUtils::GetSupportedDepthStencilFormat();

		std::vector<VkAttachmentDescription> attachmentDescriptions = GenerateAttachmentDescriptions();
		std::vector<VkAttachmentReference> colourAttachmentReferences = GenerateColourAttachmentReferences();

		VkAttachmentReference depthStencilAttachmentReference{};
		depthStencilAttachmentReference.attachment = colourAttachmentReferences.size();
		depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = colourAttachmentReferences.size();
		subpass.pColorAttachments = colourAttachmentReferences.data();
		if (hasDepthStencil) subpass.pDepthStencilAttachment = &depthStencilAttachmentReference;

		VkPipelineStageFlags stageMask = hasDepthStencil ?
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
			: VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkImageAspectFlags dstAccessMask = hasDepthStencil ?
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			: VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = stageMask;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = stageMask;
		dependency.dstAccessMask = dstAccessMask;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1; // TODO: additional subpasses!
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
		vertexInputInfo.vertexBindingDescriptionCount = 1; // TODO: increase to account for additional bindings
		vertexInputInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributes.size();
		vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VulkanUtils::VulkanTopology(m_Specification.Topology);
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE; // TODO: put this to good use?

		VkPipelineViewportStateCreateInfo viewportStateInfo{};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.scissorCount = 1;
		// set to nullptr because these are included in dynamic state:
		viewportStateInfo.pViewports = nullptr;
		viewportStateInfo.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizationStateInfo{};
		rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateInfo.depthClampEnable = VK_FALSE;
		rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE; // if true, the fragment shader is skipped (if we just want the vertex shader output in a later render pass)
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
		depthStencilStateInfo.depthTestEnable = m_Specification.FramebufferSpec.HasDepthStencil ? VK_TRUE : VK_FALSE;
		depthStencilStateInfo.depthWriteEnable = m_Specification.FramebufferSpec.HasDepthStencil ? VK_TRUE : VK_FALSE;
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

	void VulkanRenderPass::CreateFramebuffer()
	{
		m_Framebuffer = Ref<VulkanFramebuffer>::Create(m_Specification.FramebufferSpec);

		m_VkFramebuffers = m_Framebuffer->GenerateVkFramebuffers(m_RenderPass);
		
	}

	void VulkanRenderPass::DestroyFramebuffer()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		for (auto& fb : m_VkFramebuffers)
		{
			vkDestroyFramebuffer(device, fb, nullptr);
		}

		m_Framebuffer.Reset();
	}

	void VulkanRenderPass::ValidateSpecification()
	{
		// TODO: check that the framebuffer attachments make sense:
		// - Get output data from shader (more reflection!!) and check that we have the right number/type of attachments
		// - Images should all be of the correct size
		// - Images should have been created with valid usage flags/formats, and transitioned to compatible layouts
		// - If targeting swapchain, check framebuffer attachments vector empty
	}

	std::vector<VkAttachmentDescription> VulkanRenderPass::GenerateAttachmentDescriptions()
	{
		std::vector<VkAttachmentDescription> descriptions;

		bool targetSwapchain = m_Specification.FramebufferSpec.TargetSwapchain;
		auto& colourAttachments = m_Specification.FramebufferSpec.ColourAttachmentSpecs;

		for (AttachmentSpecification& attachment : colourAttachments)
		{
			VkAttachmentDescription& description = descriptions.emplace_back();
			description.format = targetSwapchain ?
				m_Swapchain->GetSwapchainImageFormat() :
				VulkanUtils::VulkanColourFormat(attachment.Format);
			description.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: multisampling
			description.loadOp = VulkanUtils::VulkanLoadOp(attachment.LoadOp);
			description.storeOp = VulkanUtils::VulkanStoreOp(attachment.StoreOp);
			description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			description.initialLayout = VulkanUtils::VulkanImageLayout(attachment.InitialLayout);
			description.finalLayout = VulkanUtils::VulkanImageLayout(attachment.FinalLayout);
		}

		if (m_Specification.FramebufferSpec.HasDepthStencil)
		{
			AttachmentSpecification& attachment = m_Specification.FramebufferSpec.DepthStencilAttachmentSpec;

			VkAttachmentDescription& description = descriptions.emplace_back();
			description.format = VulkanUtils::GetSupportedDepthStencilFormat();
			description.samples = VK_SAMPLE_COUNT_1_BIT;
			description.loadOp = VulkanUtils::VulkanLoadOp(attachment.LoadOp);
			description.storeOp = VulkanUtils::VulkanStoreOp(attachment.StoreOp);
			description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			description.initialLayout = VulkanUtils::VulkanImageLayout(attachment.InitialLayout);
			description.finalLayout = VulkanUtils::VulkanImageLayout(attachment.FinalLayout);
		}

		return descriptions;
	}

	std::vector<VkAttachmentReference> VulkanRenderPass::GenerateColourAttachmentReferences()
	{
		std::vector<VkAttachmentReference> references;

		auto& colourAttachments = m_Specification.FramebufferSpec.ColourAttachmentSpecs;
		uint32_t colourAttachmentCount = colourAttachments.size();

		for (uint32_t i = 0; i < colourAttachmentCount; i++)
		{
			auto& reference = references.emplace_back();
			reference.attachment = i;
			reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		return references;
	}

}
