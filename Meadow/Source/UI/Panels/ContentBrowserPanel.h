#pragma once

#include "UI/Elements/EditorIcons.h"
#include "Zahra/Core/Timer.h"
#include "Zahra/Events/Event.h"
#include "Zahra/Events/MouseEvent.h"
#include "Zahra/Renderer/Texture.h"

#include <filesystem>

namespace Zahra
{
	struct DirectoryData
	{
		std::filesystem::path Path;

		DirectoryData(std::filesystem::path path)
			: Path(path) {}
	};

	struct FileData
	{
		enum class ContentType
		{
			Unknown = 0,
			Image,
			Audio,
			Text,
			Scene
		};

		std::filesystem::path Path;
		ContentType Type;
		uintmax_t Size;

		FileData(std::filesystem::path path, uint32_t size)
			: Path(path), Size(size)
		{
			// TODO: fill this out with other types (might be worth making this conversion a helper function or map)
			if (path.extension().string() == ".zsc")
				Type = ContentType::Scene;
			else if (path.extension().string() == ".png") // TODO: include other formats!!
				Type = ContentType::Image;
			else
				Type = ContentType::Unknown;
		}
	};

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnLoadProject();

		void OnEvent(Event& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);

		void OnImGuiRender();

	private:
		std::filesystem::path m_ProjectRoot;
		std::filesystem::path m_CurrentPath;

		std::vector<DirectoryData> m_Subdirectories;
		std::vector<FileData> m_Files;

		std::vector<std::filesystem::path> m_ForwardStack;

		bool m_ShowAllFiles = true;

		Timer m_RefreshTimer;
		float m_RefreshPeriod = 500.0f; // in milliseconds

		struct
		{
			bool ShowHidden = false;
		}
		m_BrowserOptions;

		int32_t m_ThumbnailSize = 64;

		bool m_PanelHovered = false, m_PanelFocused = false;
		bool m_ChildHovered = false, m_ChildFocused = false;

		void DisplayNavBar();
		void DisplayCurrentDirectory_Filesystem();
		void DisplayCurrentDirectory_AssetManager();

		void GoBack();
		void GoForward();

		// TODO: run this stuff on a dedicated thread
		void ValidateCurrentDirectory();
		void ScanCurrentDirectory();
		void Refresh();

		bool DragFile(FileData file);

	};

}
