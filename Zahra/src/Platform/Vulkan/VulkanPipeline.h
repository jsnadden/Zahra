#pragma once

#include "Zahra/Renderer/Pipeline.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const PipelineSpecification& specification);
		virtual ~VulkanPipeline();

		virtual PipelineSpecification& GetSpecification() override { return m_Specification; }
		virtual const PipelineSpecification& GetSpecification() const override { return m_Specification; }

		virtual void Invalidate() override;

		virtual Ref<Shader> GetShader() const override { return m_Specification.Shader; }

	private:
		PipelineSpecification m_Specification;


		VkPipelineLayout m_PipelineLayout = nullptr;
		VkPipeline m_Pipeline = nullptr;
		VkPipelineCache m_PipelineCache = nullptr; // TODO: set this up too

	};

}
