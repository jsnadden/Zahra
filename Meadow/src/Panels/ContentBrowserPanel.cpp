#include "zpch.h"
#include "ContentBrowserPanel.h"

#include "Zahra/Core/Input.h"

#include <ImGui/imgui.h>

// TODO: later we'll get rid of this being hardcoded, in favour of a "projects" system
static const std::filesystem::path s_AssetsRoot = "assets";

namespace Zahra
{
	
	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentPath(s_AssetsRoot)
	{
		m_DirectoryIconTexture = Texture2D::Create("resources/icons/browser/DirectoryIcon.png");
		m_DefaultFileIconTexture = Texture2D::Create("resources/icons/browser/FileIcon.png");
	}

	void ContentBrowserPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressedEvent>(Z_BIND_EVENT_FN(ContentBrowserPanel::OnMouseButtonPressedEvent));
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ValidateCurrentDirectory();

		// TODO: currently scanning directories per frame, maybe make this only refresh
		// once per second(?), caching the entries (and obvs keep displaying per frame)

		ImGui::Begin("ContentBrowserPanel", 0, ImGuiWindowFlags_NoCollapse);

		m_PanelHovered = ImGui::IsWindowHovered();
		m_PanelFocused = ImGui::IsWindowFocused();

		// TODO: add the following:
		// 1) forward, back and refresh buttons (stacks!)
		// 2) rightclick context menu on blank space to create a new directory (etc.?)

		DisplayNavBar();
		DisplayCurrentDirectory();
		DisplayFileData();

		ImGui::End();
	}

	
	void ContentBrowserPanel::DisplayNavBar()
	{
		if (ImGui::Button("<-"))
		{
			if (m_CurrentPath != s_AssetsRoot) m_CurrentPath = m_CurrentPath.parent_path();
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - 200);

		ImGui::Text("Icon size:");

		ImGui::SameLine();

		if (ImGui::Button("small"))
		{
			m_ThumbnailSize = 64;
		}

		ImGui::SameLine();

		if (ImGui::Button("LARGE"))
		{
			m_ThumbnailSize = 128;
		}

		ImGui::Separator();

	}

	void ContentBrowserPanel::DisplayFileTree(std::filesystem::path filepath)
	{
		for (auto& item : std::filesystem::directory_iterator(filepath))
		{
			const std::filesystem::path& path = item.path().filename();
			std::string pathString = path.string();

			if (item.is_directory())
			{
				if (ImGui::TreeNodeEx(pathString.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
				{
					DisplayFileTree(filepath / path);

					ImGui::TreePop();
				}
			}
			else
			{
				if (ImGui::TreeNodeEx(pathString.c_str(), ImGuiTreeNodeFlags_Leaf))
				{
					ImGui::TreePop();
				}
			}

		}
	}

	void ContentBrowserPanel::DisplayCurrentDirectory()
	{
	
		int minPadding = 4;
		int minColumnWidth = (int)m_ThumbnailSize + minPadding;
		float panelWidth = ImGui::GetContentRegionAvail().x;

		int numColumns = ((int)panelWidth) / minColumnWidth;

		ImGui::Columns(numColumns > 0 ? numColumns : 1, 0, false);

		for (auto item : std::filesystem::directory_iterator(m_CurrentPath))
		{
			const std::filesystem::path& path = item.path();
			std::filesystem::path relativePath = std::filesystem::relative(path, s_AssetsRoot);
			std::string filenameString = relativePath.filename().string();

			// TODO: add rightclick context menus for directory items: rename, access metadata etc.

			if (item.is_directory())
			{
				ImGui::ImageButton((ImTextureID)m_DirectoryIconTexture->GetRendererID(), { m_ThumbnailSize, m_ThumbnailSize }, { 0,1 }, { 1,0 });
				
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					m_CurrentPath /= path.filename();
				}
			}
			else
			{
				ImGui::ImageButton((ImTextureID)m_DefaultFileIconTexture->GetRendererID(), { m_ThumbnailSize, m_ThumbnailSize }, { 0,1 }, { 1,0 });
			}

			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

		}

		ImGui::Columns(1);

	}

	void ContentBrowserPanel::DisplayFileData()
	{
		// TODO: display some data and metadata for the selected dir/file
	}

	void ContentBrowserPanel::ValidateCurrentDirectory()
	{
		while (!std::filesystem::exists(m_CurrentPath))
		{
			m_CurrentPath = m_CurrentPath.parent_path();
		}
	}

	bool ContentBrowserPanel::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		if (!m_PanelFocused) return false;

		switch (event.GetMouseButton())
		{
			case MouseCode::Button3:
			{
				if (m_CurrentPath != s_AssetsRoot)
				{
					m_CurrentPath = m_CurrentPath.parent_path();
					return true;
				}

				break;
			}
		}

		return false;

	}

}

