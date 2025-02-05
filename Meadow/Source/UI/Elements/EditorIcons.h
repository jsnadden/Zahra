#pragma once

#include "Zahra/Core/Ref.h"
#include "Zahra/ImGui/ImGuiLayer.h"
#include "Zahra/Renderer/Texture.h"

namespace Zahra
{
	class EditorIcons
	{
	public:
		static void Init();
		static void Shutdown();
		static ImGuiTextureHandle GetIconHandle(const std::string& iconName);
	};
}
