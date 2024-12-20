#pragma once

#include "Zahra/Core/Layer.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/KeyEvent.h"
#include "Zahra/Events/MouseEvent.h"
#include "Zahra/Renderer/Image.h"
#include "Zahra/Renderer/Texture.h"

namespace Zahra
{
	typedef void* ImGuiTextureHandle;

	struct ImGuiLayerConfig
	{
		bool Enabled = true;

		// colour scheme stuff?
	};

	class ImGuiLayer : public Layer
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual ImGuiTextureHandle RegisterTexture(Ref<Texture2D> texture) = 0;
		virtual void DeregisterTexture(ImGuiTextureHandle textureHandle) = 0;

		virtual void SetRenderTarget(Ref<Image2D> renderTarget) = 0;

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetColourTheme();

		static ImGuiLayer* Create();

	protected:
		bool m_BlockEvents = true;

	};
}
