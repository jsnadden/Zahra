#pragma once

#include "Zahra/Renderer/RendererAPI.h"

namespace Zahra
{

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColour(const glm::vec4& colour) override;

		virtual void Present() override;	

		// TEMPORARY
		virtual void SetPipeline(const Ref<Pipeline>& pipeline) override;

	};

}
