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

		vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
		
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);

		vkDestroyRenderPass(device, m_RenderPass, nullptr);
	}

	void VulkanRenderPass::CreateRenderPass()
	{
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();
		bool hasDepthStencil = m_Specification.RenderTarget->GetSpecification().HasDepthStencil;
		VkFormat depthStencilFormat = VulkanUtils::GetSupportedDepthStencilFormat();
		Ref<VulkanFramebuffer> renderTarget = m_Specification.RenderTarget.As<VulkanFramebuffer>();

		std::vector<VkAttachmentDescription> attachmentDescriptions = renderTarget->GetAttachmentDescriptions();

		uint32_t colourAttachmentCount = attachmentDescriptions.size();
		if (hasDepthStencil)
			colourAttachmentCount--;

		std::vector<VkAttachmentReference> colourAttachmentReferences;
		for (uint32_t i = 0; i < colourAttachmentCount; i++)
		{
			auto& attachmentReference = colourAttachmentReferences.emplace_back();
			attachmentReference.attachment = i;
			attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

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

		std::vector<VkSubpassDependency> subpassDependencies;
		if (colourAttachmentCount > 0)
		{
			// these ensure that each render pass will wait to write to its colour attachments
			// until the previous pass has finished reading all texture data for its fragment shader
			auto& previousPass = subpassDependencies.emplace_back();
			previousPass.srcSubpass = VK_SUBPASS_EXTERNAL;
			previousPass.dstSubpass = 0;
			previousPass.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			previousPass.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			previousPass.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			previousPass.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			previousPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			auto& nextPass = subpassDependencies.emplace_back();
			nextPass.srcSubpass = VK_SUBPASS_EXTERNAL;
			nextPass.dstSubpass = 0;
			nextPass.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			nextPass.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			nextPass.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			nextPass.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			nextPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		if (hasDepthStencil)
		{
			// similar to above, but for depth attachments
			auto& previousPassDepth = subpassDependencies.emplace_back();
			previousPassDepth.srcSubpass = VK_SUBPASS_EXTERNAL;
			previousPassDepth.dstSubpass = 0;
			previousPassDepth.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			previousPassDepth.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			previousPassDepth.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			previousPassDepth.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			previousPassDepth.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			auto& nextPassDepth = subpassDependencies.emplace_back();
			nextPassDepth.srcSubpass = 0;
			nextPassDepth.dstSubpass = VK_SUBPASS_EXTERNAL;
			nextPassDepth.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			nextPassDepth.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			nextPassDepth.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			nextPassDepth.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			nextPassDepth.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = subpassDependencies.size();
		renderPassInfo.pDependencies = subpassDependencies.data();

		VulkanUtils::ValidateVkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass),
			"Vulkan render pass creation failed");
	}

	void VulkanRenderPass::CreatePipeline()
	{
		Ref<VulkanShader> shader = Ref<VulkanShader>(m_Specification.Shader);
		const auto& shaderStageInfos = shader->GetPipelineShaderStageInfos();
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();
		VertexBufferLayout vertexLayout = shader->GetVertexLayout();
		std::vector<VkDescriptorSetLayout> layouts = shader->GetDescriptorSetLayouts();
		// TODO: get push constant ranges from shader reflection
		bool hasDepthStencil = m_Specification.RenderTarget->GetSpecification().HasDepthStencil;

		///////////////////////////////////////////////////////////////////////////////////////
		// Dynamic state
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		///////////////////////////////////////////////////////////////////////////////////////
		// Vertex input
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

		///////////////////////////////////////////////////////////////////////////////////////
		// Input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VulkanUtils::VulkanTopology(m_Specification.Topology);
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE; // TODO: put this to good use?

		///////////////////////////////////////////////////////////////////////////////////////
		// Viewport
		VkPipelineViewportStateCreateInfo viewportStateInfo{};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.scissorCount = 1;
		// set to nullptr because these are included in dynamic state:
		viewportStateInfo.pViewports = nullptr;
		viewportStateInfo.pScissors = nullptr;

		///////////////////////////////////////////////////////////////////////////////////////
		// Rasterisation
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

		///////////////////////////////////////////////////////////////////////////////////////
		// Multisampling
		VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
		multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.sampleShadingEnable = VK_FALSE;
		multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleStateInfo.minSampleShading = 1.0f;
		multisampleStateInfo.pSampleMask = nullptr;
		multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleStateInfo.alphaToOneEnable = VK_FALSE;

		///////////////////////////////////////////////////////////////////////////////////////
		// Depth/stencil tests
		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo{};
		depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateInfo.depthTestEnable = hasDepthStencil;
		depthStencilStateInfo.depthWriteEnable = hasDepthStencil;
		depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.minDepthBounds = 0.0f;
		depthStencilStateInfo.maxDepthBounds = 1.0f;
		depthStencilStateInfo.stencilTestEnable = VK_FALSE;
		depthStencilStateInfo.front = {};
		depthStencilStateInfo.back = {};

		///////////////////////////////////////////////////////////////////////////////////////
		// Colour blending
		// TODO: configure blending in specification
		std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachmentStates;
		for (auto& attachmentSpec : m_Specification.RenderTarget->GetSpecification().ColourAttachmentSpecs)
		{
			auto& blendState = colourBlendAttachmentStates.emplace_back();
			blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendState.blendEnable = VK_FALSE; // TODO: no translucency!!
			blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendState.colorBlendOp = VK_BLEND_OP_ADD;
			blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendState.alphaBlendOp = VK_BLEND_OP_ADD;
		}		

		VkPipelineColorBlendStateCreateInfo colourBlendState{};
		colourBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colourBlendState.logicOpEnable = VK_FALSE;
		colourBlendState.logicOp = VK_LOGIC_OP_COPY;
		colourBlendState.attachmentCount = colourBlendAttachmentStates.size();
		colourBlendState.pAttachments = colourBlendAttachmentStates.data();
		colourBlendState.blendConstants[0] = 0.0f;
		colourBlendState.blendConstants[1] = 0.0f;
		colourBlendState.blendConstants[2] = 0.0f;
		colourBlendState.blendConstants[3] = 0.0f;

		///////////////////////////////////////////////////////////////////////////////////////
		// Pipeline layout creation (describes expected shader resources)
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0; // TODO: push constants
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VulkanUtils::ValidateVkResult(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout),
			"Vulkan pipeline layout creation failed");

		///////////////////////////////////////////////////////////////////////////////////////
		// Pipeline creation
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
		pipelineInfo.pColorBlendState = &colourBlendState;
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
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();
		Ref<VulkanFramebuffer> renderTarget = m_Specification.RenderTarget.As<VulkanFramebuffer>();
		std::vector<VkImageView> attachmentImageViews = renderTarget->GetImageViews();

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = attachmentImageViews.size();
		framebufferInfo.pAttachments = attachmentImageViews.data();
		framebufferInfo.width = renderTarget->GetSpecification().Width;
		framebufferInfo.height = renderTarget->GetSpecification().Height;
		framebufferInfo.layers = 1; // TODO: set this in FramebufferSpec

		VulkanUtils::ValidateVkResult(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffer),
			"Vulkan framebuffer creation failed");
	}

	void VulkanRenderPass::ValidateSpecification()
	{
		// TODO: check that the framebuffer attachments make sense:
		// - Compare shader reflection data (fragment stage outputs) with the target framebuffer,
		// - Check that we have the right number/type of attachments
		// - Images should all be of the correct size
		// - Images should have been created with valid usage flags/formats, and transitioned to compatible layouts
	}

}
