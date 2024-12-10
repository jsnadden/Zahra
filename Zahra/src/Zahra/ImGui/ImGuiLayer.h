#pragma once

#include "Zahra/Core/Layer.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/KeyEvent.h"
#include "Zahra/Events/MouseEvent.h"
#include "Zahra/Renderer/Texture.h"

namespace Zahra
{
	typedef void* ImGuiResourceHandle;

	struct ImGuiLayerConfig
	{
		bool Enabled = true;
		bool ClearSwapchain = false;
	};

	class ImGuiLayer : public Layer
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual ImGuiResourceHandle RegisterTexture(Ref<Texture2D> texture) = 0;
		virtual void DeregisterTexture(ImGuiResourceHandle textureHandle) = 0;

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetColourTheme(); // TODO: options for light/dark modes? (De)Serialising `themes` from a .yml or .ini? This feels out of scope...

		static ImGuiLayer* Create();

	protected:
		bool m_BlockEvents = true;

	};
}
