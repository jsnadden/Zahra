#pragma once

#include "Zahra/Core/Layer.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/KeyEvent.h"
#include "Zahra/Events/MouseEvent.h"
#include "Zahra/Renderer/Texture.h"

namespace Zahra
{
	class ImGuiLayer : public Layer
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void* RegisterTexture(Ref<Texture2D> texture) = 0;

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetColourTheme(); // TODO: make a "theme" struct and pass one in here. Also save the theme to a .yml?

		static ImGuiLayer* Create();

	protected:
		bool m_BlockEvents = true;

	};
}
