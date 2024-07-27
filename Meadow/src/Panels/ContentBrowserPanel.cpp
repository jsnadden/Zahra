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
		m_RefreshTimer.Reset();

		m_Icons["DirectoryThumb"] = Texture2D::Create("resources/icons/browser/folder.png");
		m_Icons["DefaultFileThumb"] = Texture2D::Create("resources/icons/browser/blank_file.png");
		m_Icons["Back"] = Texture2D::Create("resources/icons/browser/back_arrow.png");
		m_Icons["Forward"] = Texture2D::Create("resources/icons/browser/forward_arrow.png");
		//m_Icons["Refresh"] = Texture2D::Create("resources/icons/browser/refresh.png");
		//m_Icons["Drag"] = Texture2D::Create("resources/icons/browser/drag_indicator.png");
	}

	void ContentBrowserPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressedEvent>(Z_BIND_EVENT_FN(ContentBrowserPanel::OnMouseButtonPressedEvent));
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		if (m_RefreshTimer.ElapsedMillis() > m_RefreshPeriod) Refresh();

		ImGui::Begin("Content Browser", 0, ImGuiWindowFlags_NoCollapse);

		m_PanelHovered = ImGui::IsWindowHovered();
		m_PanelFocused = ImGui::IsWindowFocused();

		// TODO: rightclick context menu on blank space to create a new directory etc.
		
		DisplayNavBar();
		DisplayCurrentDirectory();
		DisplayFileData();

		ImGui::End();
	}
	
	void ContentBrowserPanel::DisplayNavBar()
	{
		float iconSize = 20.f;
		std::string thumbSizeString = "Thumbnail size";
		float thumbSizeStringLength = ImGui::CalcTextSize(thumbSizeString.c_str()).x;

		if (ImGui::BeginTable("NavBar", 7, ImGuiTableColumnFlags_NoResize))
		{

			ImGui::TableSetupColumn("back", ImGuiTableColumnFlags_WidthFixed, 1.5f * iconSize);
			ImGui::TableSetupColumn("forward", ImGuiTableColumnFlags_WidthFixed, 1.5f * iconSize);
			ImGui::TableSetupColumn("spacer1", ImGuiTableColumnFlags_WidthFixed, 50.f);
			ImGui::TableSetupColumn("currentdirname");
			ImGui::TableSetupColumn("spacer2", ImGuiTableColumnFlags_WidthFixed, 50.f);
			ImGui::TableSetupColumn("thumbsizetext", ImGuiTableColumnFlags_WidthFixed, thumbSizeStringLength + 4.f);
			ImGui::TableSetupColumn("thumbsizeslider", ImGuiTableColumnFlags_WidthFixed, 100.f);

			ImGui::TableNextColumn();
			if (ImGui::ImageButton((ImTextureID)m_Icons["Back"]->GetRendererID(), { iconSize, iconSize }, { 0,1 }, { 1,0 }))
			{
				GoBack();
			}

			ImGui::TableNextColumn();
			if (ImGui::ImageButton((ImTextureID)m_Icons["Forward"]->GetRendererID(), { iconSize, iconSize }, { 0,1 }, { 1,0 }))
			{
				GoForward();
			}

			ImGui::TableNextColumn();

			ImGui::TableNextColumn();
			{
				std::string currentDirName = m_CurrentPath.string();

				// this necessitates a max tag size of 255 ascii characters (plus null terminator)
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strcpy_s(buffer, currentDirName.c_str());

				ImGui::PushItemWidth(ImGui::GetColumnWidth());
				ImGui::InputText("##direxplorer", buffer, sizeof(buffer)); // TODO: navigate to directory by inputting the path
				ImGui::PopItemWidth();

			}

			ImGui::TableNextColumn();

			ImGui::TableNextColumn();
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(thumbSizeString.c_str());
			}

			ImGui::TableNextColumn();
			{
				ImGui::PushItemWidth(ImGui::GetColumnWidth());
				ImGui::SliderInt("##ThumbnailSize", &m_ThumbnailSize, 64, 256, "");
				ImGui::PopItemWidth();
			}

			ImGui::EndTable();
		}

		ImGui::Separator();
	}

	void ContentBrowserPanel::DisplayCurrentDirectory()
	{	
		ImGui::BeginChild("DisplayDirContents");

		m_ChildHovered = ImGui::IsWindowHovered();
		m_ChildFocused = ImGui::IsWindowFocused();

		int buttonPadding = 2 * (int)ImGui::GetStyle().FramePadding.x;
		int minColumnWidth = m_ThumbnailSize + buttonPadding;
		float panelWidth = ImGui::GetContentRegionAvail().x;

		int numColumns = std::max(((int)panelWidth) / minColumnWidth, 1);

		if (ImGui::BeginTable("DirThumbs", numColumns))
		{
			// TODO: add rightclick context menus for directory items: rename, access metadata etc.

			for (auto dir : m_Subdirectories)
			{
				ImGui::TableNextColumn();

				const std::filesystem::path& path = dir.Path;
				std::string filenameString = path.filename().string();

				ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
				ImGui::ImageButton(filenameString.c_str(), (ImTextureID)m_Icons["DirectoryThumb"]->GetRendererID(), {(float)m_ThumbnailSize, (float)m_ThumbnailSize}, {0,1}, {1,0});
				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				{
					ImGui::BeginTooltip();
					ImGui::Text(filenameString.c_str());
					ImGui::EndTooltip();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					m_ForwardStack.clear();
					m_CurrentPath /= filenameString;
					Refresh();
				}

				ImGui::Text(filenameString.c_str());
			}

			for (auto file : m_Files)
			{
				ImGui::TableNextColumn();

				const std::filesystem::path& path = file.Path;
				std::string filenameString = path.filename().string();

				// TODO: check extension and metadata to choose a specific thumbnail
				ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
				ImGui::ImageButton((ImTextureID)m_Icons["DefaultFileThumb"]->GetRendererID(), { (float)m_ThumbnailSize, (float)m_ThumbnailSize }, { 0,1 }, { 1,0 });
				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				{
					ImGui::BeginTooltip();
					ImGui::Text(filenameString.c_str());
					ImGui::EndTooltip();
				}

				ImGui::Text(filenameString.c_str());
			}

			ImGui::EndTable();
		}

		ImGui::EndChild();
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

	void ContentBrowserPanel::ScanCurrentDirectory()
	{
		for (auto item : std::filesystem::directory_iterator(m_CurrentPath))
		{
			const std::filesystem::path& path = item.path();

			if (item.is_directory())
			{
				m_Subdirectories.emplace_back(path);
			}
			else
			{
				m_Files.emplace_back(path, std::filesystem::file_size(path));
			}
		}
	}

	void ContentBrowserPanel::Refresh()
	{
		m_Subdirectories.clear();
		m_Files.clear();

		ValidateCurrentDirectory();
		ScanCurrentDirectory();
		m_RefreshTimer.Reset();
	}

	void ContentBrowserPanel::GoBack()
	{
		if (m_CurrentPath != s_AssetsRoot)
		{
			m_ForwardStack.push_back(m_CurrentPath);
			m_CurrentPath = m_CurrentPath.parent_path();
			Refresh();
		}
	}

	void ContentBrowserPanel::GoForward()
	{
		if (!m_ForwardStack.empty())
		{
			m_CurrentPath = m_ForwardStack.back();
			m_ForwardStack.pop_back();
			Refresh();
		}
	}

	bool ContentBrowserPanel::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		if (!m_ChildFocused && !m_PanelFocused) return false;

		switch (event.GetMouseButton())
		{
			case MouseCode::Button3:
			{
				GoBack();
				return true;
			}

			case MouseCode::Button4:
			{
				GoForward();
				return true;
			}

			default:
				break;
		}

		return false;

	}

}

