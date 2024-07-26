#pragma once

#include "Zahra/Events/Event.h"
#include "Zahra/Events/MouseEvent.h"
#include "Zahra/Renderer/Texture.h"

#include <filesystem>


namespace Zahra
{

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnEvent(Event& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);

		void OnImGuiRender();

		void DisplayNavBar();
		void DisplayFileTree(std::filesystem::path filepath);
		void DisplayCurrentDirectory();
		void DisplayFileData();

	private:
		std::filesystem::path m_CurrentPath;

		Ref<Texture2D> m_DirectoryIconTexture, m_DefaultFileIconTexture;

		float m_ThumbnailSize = 64;

		bool m_PanelHovered = false;
		bool m_PanelFocused = false;

		void ValidateCurrentDirectory();
	};

}
