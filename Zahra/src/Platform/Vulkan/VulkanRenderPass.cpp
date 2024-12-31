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
		static VkAttachmentLoadOp VulkanLoadOp(AttachmentLoadOp loadOp)
		{
			switch (loadOp)
			{
				case AttachmentLoadOp::Unspecified:		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				case AttachmentLoadOp::Clear:			return VK_ATTACHMENT_LOAD_OP_CLEAR;
				case AttachmentLoadOp::Load:			return VK_ATTACHMENT_LOAD_OP_LOAD;
			}
			Z_CORE_ASSERT(false, "Unsupported load operation");
			return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
		}

		static VkImageLayout InitialLayoutFromLoadOp(AttachmentLoadOp loadOp, bool depthStencil)
		{
			if (loadOp == AttachmentLoadOp::Clear)
				return VK_IMAGE_LAYOUT_UNDEFINED;
			else if (depthStencil)
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // TODO: framebuffer spec should flag whether we'll sample the depth/stencil attachment or not
			else
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
		m_TargetSwapchain = !m_Specification.RenderTarget;

		CreateRenderPass();
		CreatePipeline();
		CreateFramebuffers();

		CreateClearData();
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		VkDevice& device = m_Swapchain->GetVkDevice();
		vkDeviceWaitIdle(device);

		DestroyFramebuffers();
		
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);

		vkDestroyRenderPass(device, m_RenderPass, nullptr);
	}

	const Ref<Framebuffer> VulkanRenderPass::GetRenderTarget() const
	{
		Z_CORE_ASSERT(!m_TargetSwapchain, "Do not call this method for swapchain-targetting render passes");

		return m_Specification.RenderTarget;
	}

	bool VulkanRenderPass::TargetSwapchain()
	{
		return m_TargetSwapchain;
	}

	const VkFramebuffer& VulkanRenderPass::GetVkFramebuffer() const
	{
		if (m_TargetSwapchain)
			return m_Framebuffers[m_Swapchain->GetImageIndex()];
		else
			return m_Framebuffers[0];

	}

	const std::vector<VkClearValue> VulkanRenderPass::GetClearValues() const
	{
		if (m_TargetSwapchain)
		{
			return {{ 0.0f, 0.0f, 1.0f, 1.0f }};
		}
		else
		{
			return m_Specification.RenderTarget.As<VulkanFramebuffer>()->GetClearValues();
		}
	}

	void VulkanRenderPass::OnResize()
	{
		DestroyFramebuffers();
		CreateFramebuffers();

		for (auto& rect : m_ClearRects)
		{
			rect.rect.extent.width = m_TargetSwapchain ? m_Swapchain->GetWidth() : m_Specification.RenderTarget->GetWidth();
			rect.rect.extent.height = m_TargetSwapchain ? m_Swapchain->GetHeight() : m_Specification.RenderTarget->GetHeight();
		}
	}

	void VulkanRenderPass::CreateRenderPass()
	{
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();

		if (m_TargetSwapchain)
		{
			VkAttachmentDescription description{};
			description.flags = 0;
			description.format = m_Swapchain->GetSwapchainImageFormat();
			description.samples = VK_SAMPLE_COUNT_1_BIT;
			description.loadOp = m_Specification.ClearColourAttachments ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference reference{};
			reference.attachment = 0;
			reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &reference;

			// this subpass dependency sets up waiting until the swapchain has finished
			// reading the colour attachment (for presentation) before the next frame is written to it
			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &description;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			VulkanUtils::ValidateVkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass),
				"Vulkan swapchain render pass creation failed");
		}
		else
		{
			bool hasDepthStencil = m_Specification.RenderTarget->GetSpecification().HasDepthStencil;
			VkFormat depthStencilFormat = VulkanUtils::GetSupportedDepthStencilFormat();
			Ref<VulkanFramebuffer> renderTarget = m_Specification.RenderTarget.As<VulkanFramebuffer>();

			///////////////////////////////////////////////////////////////////////////////////////
			// Attachment descriptions
			std::vector<VkAttachmentDescription> attachmentDescriptions = renderTarget->GetAttachmentDescriptions();
			uint32_t colourAttachmentCount = (uint32_t)attachmentDescriptions.size();
			if (hasDepthStencil)
				colourAttachmentCount--;

			for (uint32_t i = 0; i < colourAttachmentCount; i++)
			{
				attachmentDescriptions[i].loadOp = m_Specification.ClearColourAttachments ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachmentDescriptions[i].initialLayout = m_Specification.ClearColourAttachments ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			if (hasDepthStencil)
			{
				attachmentDescriptions[colourAttachmentCount].loadOp = m_Specification.ClearDepthAttachment ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachmentDescriptions[colourAttachmentCount].initialLayout = m_Specification.ClearDepthAttachment ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}

			///////////////////////////////////////////////////////////////////////////////////////
			// Attachment references
			std::vector<VkAttachmentReference> colourAttachmentReferences;
			for (uint32_t i = 0; i < colourAttachmentCount; i++)
			{
				auto& attachmentReference = colourAttachmentReferences.emplace_back();
				attachmentReference.attachment = i;
				attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			VkAttachmentReference depthStencilAttachmentReference{};
			depthStencilAttachmentReference.attachment = (uint32_t)colourAttachmentReferences.size();
			depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			///////////////////////////////////////////////////////////////////////////////////////
			// Subpass
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (uint32_t)colourAttachmentReferences.size();
			subpass.pColorAttachments = colourAttachmentReferences.data();
			if (hasDepthStencil) subpass.pDepthStencilAttachment = &depthStencilAttachmentReference;

			///////////////////////////////////////////////////////////////////////////////////////
			// Subpass dependencies
			VkPipelineStageFlags stageMask = hasDepthStencil ?
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
				: VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			// the following subpass dependencies set up synchronisation barriers within Vulkan's command
			// queue execution, so that each render pass may use the output attachments of the previous
			// one as a sampled texture input to their fragment shader stage
			std::vector<VkSubpassDependency> subpassDependencies;
			if (colourAttachmentCount > 0)
			{
				// this ensures that each render pass will wait to write to its colour attachments,
				// until all previous passes have finished reading texture data into their fragment shaders
				auto& previousPass = subpassDependencies.emplace_back();
				previousPass.srcSubpass = VK_SUBPASS_EXTERNAL;
				previousPass.dstSubpass = 0;
				previousPass.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				previousPass.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				previousPass.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				previousPass.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				previousPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				// this ensures that each render pass will wait to read texture data into its fragment
				// shader, until all previous passes have finished writing to their colour attachments
				auto& nextPass = subpassDependencies.emplace_back();
				nextPass.srcSubpass = 0;
				nextPass.dstSubpass = VK_SUBPASS_EXTERNAL;
				nextPass.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				nextPass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				nextPass.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				nextPass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				nextPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			if (hasDepthStencil)
			{
				// similar to above
				auto& previousPassDepth = subpassDependencies.emplace_back();
				previousPassDepth.srcSubpass = VK_SUBPASS_EXTERNAL;
				previousPassDepth.dstSubpass = 0;
				previousPassDepth.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				previousPassDepth.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				previousPassDepth.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				previousPassDepth.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				previousPassDepth.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				auto& nextPassDepth = subpassDependencies.emplace_back();
				nextPassDepth.srcSubpass = 0;
				nextPassDepth.dstSubpass = VK_SUBPASS_EXTERNAL;
				nextPassDepth.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				nextPassDepth.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				nextPassDepth.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				nextPassDepth.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				nextPassDepth.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			///////////////////////////////////////////////////////////////////////////////////////
			// Render pass creation
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
			renderPassInfo.pAttachments = attachmentDescriptions.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = (uint32_t)subpassDependencies.size();
			renderPassInfo.pDependencies = subpassDependencies.data();

			VulkanUtils::ValidateVkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass),
				"Vulkan render pass creation failed");
		}
	}

	void VulkanRenderPass::CreatePipeline()
	{
		Ref<VulkanShader> shader = Ref<VulkanShader>(m_Specification.Shader);

		if (!shader)
			return;

		const auto& shaderStageInfos = shader->GetPipelineShaderStageInfos();
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();
		VertexBufferLayout vertexLayout = shader->GetVertexLayout();
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = shader->GetDescriptorSetLayouts();
		// TODO: get push constant ranges from shader reflection

		bool hasDepthStencil = false;
		if (!m_TargetSwapchain)
			hasDepthStencil = m_Specification.RenderTarget->GetSpecification().HasDepthStencil;

		///////////////////////////////////////////////////////////////////////////////////////
		// Dynamic state
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		
		if (m_Specification.DynamicLineWidths)
			dynamicStates.emplace_back(VK_DYNAMIC_STATE_LINE_WIDTH);

		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		///////////////////////////////////////////////////////////////////////////////////////
		// Vertex input
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
		// TODO: should really gather this info in shader, but for now we'll either
		// have no bindings (i.e no vertex buffers), or just one at index 0
		if (vertexLayout.GetElementCount())
		{
			VkVertexInputBindingDescription& vertexInputBinding = vertexInputBindingDescriptions.emplace_back();
			vertexInputBinding.binding = 0;
			vertexInputBinding.stride = vertexLayout.GetStride();
			vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}

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
				vertexInputAttributes[location].offset = (uint32_t)element.Offset;

				location++;
			}

			binding++;
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)vertexInputBindingDescriptions.size();
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
		if (m_TargetSwapchain)
		{
			auto& blendState = colourBlendAttachmentStates.emplace_back();
			blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendState.blendEnable = VK_FALSE;
			blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendState.colorBlendOp = VK_BLEND_OP_ADD;
			blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendState.alphaBlendOp = VK_BLEND_OP_ADD;
		}
		else
		{
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
		}

		VkPipelineColorBlendStateCreateInfo colourBlendState{};
		colourBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colourBlendState.logicOpEnable = VK_FALSE;
		colourBlendState.logicOp = VK_LOGIC_OP_COPY;
		colourBlendState.attachmentCount = (uint32_t)colourBlendAttachmentStates.size();
		colourBlendState.pAttachments = colourBlendAttachmentStates.data();
		colourBlendState.blendConstants[0] = 0.0f;
		colourBlendState.blendConstants[1] = 0.0f;
		colourBlendState.blendConstants[2] = 0.0f;
		colourBlendState.blendConstants[3] = 0.0f;

		///////////////////////////////////////////////////////////////////////////////////////
		// Pipeline layout creation (expected shader resources)
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
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

		//Z_CORE_TRACE("Vulkan graphics pipeline creation successful");
	}

	void VulkanRenderPass::CreateFramebuffers()
	{
		VkDevice& device = m_Swapchain->GetDevice()->GetVkDevice();

		if (m_TargetSwapchain)
		{
			std::vector<VkImageView> swapchainImageViews = m_Swapchain->GetSwapchainImageViews();

			for (auto& imageView : swapchainImageViews)
			{
				auto& framebuffer = m_Framebuffers.emplace_back();

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = m_RenderPass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = &imageView;
				framebufferInfo.width = m_Swapchain->GetWidth();
				framebufferInfo.height = m_Swapchain->GetHeight();
				framebufferInfo.layers = 1;

				VulkanUtils::ValidateVkResult(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer),
					"Vulkan swapchain framebuffer creation failed");
			}
		}
		else
		{
			Ref<VulkanFramebuffer> renderTarget = m_Specification.RenderTarget.As<VulkanFramebuffer>();
			std::vector<VkImageView> attachmentImageViews = renderTarget->GetImageViews();
			auto& framebuffer = m_Framebuffers.emplace_back();

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = (uint32_t)attachmentImageViews.size();
			framebufferInfo.pAttachments = attachmentImageViews.data();
			framebufferInfo.width = renderTarget->GetWidth();
			framebufferInfo.height = renderTarget->GetHeight();
			framebufferInfo.layers = 1; // TODO: set this in FramebufferSpec

			VulkanUtils::ValidateVkResult(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer),
				"Vulkan framebuffer creation failed");
		}
		
	}

	void VulkanRenderPass::DestroyFramebuffers()
	{
		VkDevice& device = m_Swapchain->GetVkDevice();
		vkDeviceWaitIdle(device);

		for (auto& framebuffer : m_Framebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		m_Framebuffers.clear();
	}

	void VulkanRenderPass::CreateClearData()
	{
		uint32_t colourAttachmentCount = m_TargetSwapchain ? 1 : m_Specification.RenderTarget->GetColourAttachmentCount();
		m_ClearValues = GetClearValues();
		m_ClearAttachments.resize(colourAttachmentCount);
		m_ClearRects.resize(colourAttachmentCount);

		uint32_t renderTargetWidth = m_TargetSwapchain ? m_Swapchain->GetWidth() : m_Specification.RenderTarget->GetWidth();
		uint32_t renderTargetHeight = m_TargetSwapchain ? m_Swapchain->GetHeight() : m_Specification.RenderTarget->GetHeight();


		for (uint32_t i = 0; i < colourAttachmentCount; i++)
		{
			m_ClearAttachments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			m_ClearAttachments[i].colorAttachment = i;
			m_ClearAttachments[i].clearValue = m_ClearValues[i];

			m_ClearRects[i].baseArrayLayer = 0;
			m_ClearRects[i].layerCount = 1; // TODO: get layer count
			m_ClearRects[i].rect.extent.width = renderTargetWidth;
			m_ClearRects[i].rect.extent.height = renderTargetHeight;
			m_ClearRects[i].rect.offset.x = 0;
			m_ClearRects[i].rect.offset.y = 0;
		}

		if (!m_TargetSwapchain && m_Specification.RenderTarget->GetSpecification().HasDepthStencil)
		{
			auto& depthAttachment = m_ClearAttachments.emplace_back();
			depthAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			depthAttachment.clearValue = m_ClearValues[colourAttachmentCount];

			auto& depthRect = m_ClearRects.emplace_back();
			depthRect.baseArrayLayer = 0;
			depthRect.layerCount = 1; // TODO: get layer count
			depthRect.rect.extent.width = m_Specification.RenderTarget->GetWidth();
			depthRect.rect.extent.height = m_Specification.RenderTarget->GetHeight();
			depthRect.rect.offset.x = 0;
			depthRect.rect.offset.y = 0;
		}
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
