#include "zpch.h"
#include "SceneRenderer.h"

namespace Zahra
{

	SceneRenderer::SceneRenderer(const SceneRendererSpecification& specification)
		: m_Specification(specification)
	{
		Z_CORE_ASSERT(m_Specification.RenderTarget, "Must provide SceneRenderer with a valid render target");
		
		Renderer2DSpecification renderer2DSpec{};
		renderer2DSpec.RenderTarget = m_Specification.RenderTarget;
		m_Renderer2D = Ref<Renderer2D>::Create(renderer2DSpec);

		m_ViewportWidth = m_Specification.RenderTarget->GetWidth();
		m_ViewportHeight = m_Specification.RenderTarget->GetHeight();
	}

	SceneRenderer::~SceneRenderer()
	{

	}

	void SceneRenderer::OnViewportResize(uint32_t width, uint32_t height)
	{

	}

}
