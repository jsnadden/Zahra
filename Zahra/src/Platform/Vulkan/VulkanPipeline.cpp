#include "zpch.h"
#include "VulkanPipeline.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanUtils.h"

namespace Zahra
{
	static VkFormat ShaderDataTypeToVulkanFormat(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Int:       return VK_FORMAT_R32_SINT;
			case ShaderDataType::Int2:      return VK_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:      return VK_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:      return VK_FORMAT_R32G32B32A32_SINT;

			case ShaderDataType::Float:     return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2:    return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		Z_CORE_ASSERT(false);
		return VK_FORMAT_UNDEFINED;
	}
	
	VulkanPipeline::VulkanPipeline(const PipelineSpecification& specification)
		: m_Specification(specification)
	{
		Z_CORE_ASSERT(specification.Shader);

		Invalidate();
	}

	VulkanPipeline::~VulkanPipeline()
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->LogicalDevice;

		vkDeviceWaitIdle(device);

		vkDestroyPipeline(device, m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
	}

	void VulkanPipeline::Invalidate()
	{
		/////////////////////////////////////////////////////////////////////////////////////////////
		// GATHER DATA
		Ref<VulkanShader> shader = Ref<VulkanShader>(m_Specification.Shader);
		const auto& shaderStageInfos = shader->GetPipelineShaderStageInfos();
		Ref<VulkanSwapchain> swapchain = VulkanContext::Get()->GetSwapchain();
		VkDevice device = swapchain->GetDevice()->LogicalDevice;
		VkRenderPass renderPass = swapchain->GetVkRenderPass();
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
				vertexInputAttributes[location].format = ShaderDataTypeToVulkanFormat(element.Type);
				vertexInputAttributes[location].offset = element.Offset;

				location++;
			}

			binding++;
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1; // TODO: increase to account for additional buffers
		vertexInputInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributes.size();
		vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

		/////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIGURE PRIMITIVE ASSEMBLY
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // TODO: fill this out based on m_Specification.Topology
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE; // TODO: figure out what this should be set to

		/////////////////////////////////////////////////////////////////////////////////////////////
		// SET VIEWPORT/SCISSOR STATE (IF NOT DYNAMIC)
		//VkExtent2D extent = swapchain->GetExtent();
		//VkViewport viewport{};
		//viewport.x = 0.0f;
		//viewport.y = 0.0f;
		//viewport.width = (float)extent.width;
		//viewport.height = (float)extent.height;
		//viewport.minDepth = 0.0f;
		//viewport.maxDepth = 1.0f;
		//// TODO: fill these out based on m_Specification.TargetFramebuffer and so forth

		//VkRect2D scissor{};
		//scissor.offset = { 0, 0 };
		//scissor.extent = extent;

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
		rasterizationStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // CW oriented, because screen space coords are left-handed ugh...
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
		// TODO: fill out a VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo{};
		// (for now we'll just use a nullptr in its place)

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
		pipelineInfo.pDepthStencilState = nullptr; // TODO: include depth buffer!!
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;
		
		VulkanUtils::ValidateVkResult(vkCreateGraphicsPipelines(device, m_PipelineCache, 1, &pipelineInfo, nullptr, &m_Pipeline),
			"Vulkan graphics pipeline creation failed");

		Z_CORE_TRACE("Vulkan graphics pipeline creation successful");

	}

}
