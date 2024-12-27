#pragma once

#include "Zahra/Renderer/Renderer2D.h"
#include "Zahra/Scene/Scene.h"

namespace Zahra
{

	struct SceneRendererSpecification
	{
		Ref<Framebuffer> RenderTarget;
	};

	class SceneRenderer : public RefCounted
	{
	public:
		SceneRenderer(const SceneRendererSpecification& specification);
		~SceneRenderer();

		void OnViewportResize(uint32_t width, uint32_t height);

	private:
		SceneRendererSpecification m_Specification;
		Ref<Renderer2D> m_Renderer2D;
		uint32_t m_ViewportWidth, m_ViewportHeight;
	};

}

