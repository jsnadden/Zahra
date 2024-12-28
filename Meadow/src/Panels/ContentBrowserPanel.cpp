#include "zpch.h"
#include "ContentBrowserPanel.h"

#include "Zahra/Core/Input.h"
#include "Zahra/ImGui/ImGuiLayer.h"

#include <ImGui/imgui.h>

// TODO: later we'll get rid of this being hardcoded, in favour of a "projects" system
static const std::filesystem::path s_AssetsRoot = "Assets";

namespace Zahra
{

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentPath(s_AssetsRoot)
	{
		m_RefreshTimer.Reset();

		Texture2DSpecification textureSpec{};
		m_Icons["DirectoryThumb"]	= Texture2D::CreateFromFile(textureSpec, "Resources/Icons/Browser/folder.png");
		m_Icons["DefaultFileThumb"] = Texture2D::CreateFromFile(textureSpec, "Resources/Icons/Browser/blank_file.png");
		m_Icons["BrokenImage"]		= Texture2D::CreateFromFile(textureSpec, "Resources/Icons/Browser/broken_image.png");
		m_Icons["Back"]				= Texture2D::CreateFromFile(textureSpec, "Resources/Icons/Browser/back_arrow.png");
		m_Icons["Forward"]			= Texture2D::CreateFromFile(textureSpec, "Resources/Icons/Browser/forward_arrow.png");

		for (auto& [name, texture] : m_Icons)
		{
			m_IconHandles[name] = ImGuiLayer::GetOrCreate()->RegisterTexture(texture);
		}
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

		ImGui::End();
	}

	void ContentBrowserPanel::DisplayNavBar()
	{
		float iconSize = 25.f;
		std::string thumbSizeString = "Thumbnail size";
		float thumbSizeStringLength = ImGui::CalcTextSize(thumbSizeString.c_str()).x;

		if (ImGui::BeginTable("NavBar", 7, ImGuiTableColumnFlags_NoResize))
		{
			// TABLE SETUP
			{
				ImGui::TableSetupColumn("back", ImGuiTableColumnFlags_WidthFixed, 1.5f * iconSize);
				ImGui::TableSetupColumn("forward", ImGuiTableColumnFlags_WidthFixed, 1.5f * iconSize);
				ImGui::TableSetupColumn("spacer1", ImGuiTableColumnFlags_WidthFixed, 204.f + thumbSizeStringLength - 3.0f * iconSize); // lolol I should automate this :')
				ImGui::TableSetupColumn("selectioninfo");
				ImGui::TableSetupColumn("spacer2", ImGuiTableColumnFlags_WidthFixed, 100.f);
				ImGui::TableSetupColumn("thumbsizetext", ImGuiTableColumnFlags_WidthFixed, thumbSizeStringLength + 4.f);
				ImGui::TableSetupColumn("thumbsizeslider", ImGuiTableColumnFlags_WidthFixed, 100.f);
			}

			ImGui::TableNextColumn();

			// BACK BUTTON
			ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
			if (ImGui::ImageButton(m_IconHandles["Back"],
				{ iconSize, iconSize }, { 0,0 }, { 1,1 }))
			{
				GoBack();
			}

			ImGui::TableNextColumn();

			// FORWARD BUTTON
			if (ImGui::ImageButton(m_IconHandles["Forward"],
				{ iconSize, iconSize }, { 0,0 }, { 1,1 }))
			{
				GoForward();
			}
			ImGui::PopStyleColor();

			ImGui::TableNextColumn();
			ImGui::TableNextColumn();

			// DISPLAY DIRECTORY NAME
			{
				std::string currentDirName = m_CurrentPath.filename().string();

				float stringWidth = ImGui::CalcTextSize(currentDirName.c_str()).x;
				float indentation = .5f * (ImGui::GetColumnWidth() - stringWidth);
				indentation = std::max(indentation, .0f);

				ImGui::SameLine(indentation);

				ImGuiIO& io = ImGui::GetIO();
				auto boldFont = io.Fonts->Fonts[1];

				ImGui::AlignTextToFramePadding();
				ImGui::PushFont(boldFont);
				ImGui::Text(currentDirName.c_str());
				ImGui::PopFont();
			}

			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			
			// RESIZE ICONS
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(thumbSizeString.c_str());
				
				ImGui::TableNextColumn();
				
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

				ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
				ImGui::ImageButton(filenameString.c_str(), m_IconHandles["DirectoryThumb"],
					{ (float)m_ThumbnailSize, (float)m_ThumbnailSize }, { 0, 0 }, { 1, 1 });
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

				ImGui::PushID(filenameString.c_str());

				// TODO: check extension and metadata to choose a specific thumbnail (e.g. add screenshot to SceneSerialiser)
				ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
				switch (file.Type)
				{
					case FileData::ContentType::Image:
					{
						// TODO: get a thumbnail from file metadata!!
						ImGui::ImageButton(m_IconHandles["BrokenImage"],
							{ (float)m_ThumbnailSize, (float)m_ThumbnailSize }, { 0, 0 }, { 1, 1 });
						break;
					}
					default:
					{
						ImGui::ImageButton(m_IconHandles["DefaultFileThumb"],
							{ (float)m_ThumbnailSize, (float)m_ThumbnailSize }, { 0, 0 }, { 1, 1 });
						break;
					}
				}
				ImGui::PopStyleColor();

				bool dragged = DragFile(file);

				if (!dragged && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				{
					ImGui::BeginTooltip();
					ImGui::Text(filenameString.c_str());
					ImGui::EndTooltip();
				}

				ImGui::Text(filenameString.c_str());

				ImGui::PopID();
			}

			ImGui::EndTable();
		}

		ImGui::EndChild();
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

	bool ContentBrowserPanel::DragFile(FileData file)
	{
		std::string filepath = file.Path.string();
		std::string filename = file.Path.filename().string();

		bool dragged = false;

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_PayloadAutoExpire))
		{
			
			switch (file.Type)
			{
				case (FileData::ContentType::Scene):
				{
					Z_ASSERT(filepath.length() < 256, "Currently only support filenames up to 256 characters (including extension + null terminator)");
					ImGui::SetDragDropPayload("BROWSER_FILE_SCENE", (void *)filepath.c_str(), sizeof(char) * (filepath.length()+1), ImGuiCond_Always);
					ImGui::Text(filename.c_str());
					break;
				}
				case (FileData::ContentType::Image):
				{
					Z_ASSERT(filepath.length() < 256, "Currently only support filenames up to 256 characters (including extension + null terminator)");
					ImGui::SetDragDropPayload("BROWSER_FILE_IMAGE", (void*)filepath.c_str(), sizeof(char) * (filepath.length() + 1), ImGuiCond_Always);
					// TODO: get thumbnail from metadata
					ImGui::Image(m_IconHandles["BrokenImage"],
						{ (float)m_ThumbnailSize, (float)m_ThumbnailSize }, { 0, 0 }, { 1, 1 });
					break;
				}
				default:
				{
					ImGui::Image(m_IconHandles["DefaultFileThumb"],
						{ (float)m_ThumbnailSize, (float)m_ThumbnailSize }, { 0, 0 }, { 1, 1 });
					break;
				}
			}

			dragged = true;

			ImGui::EndDragDropSource();
		}

		return dragged;
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

	void ContentBrowserPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressedEvent>(Z_BIND_EVENT_FN(ContentBrowserPanel::OnMouseButtonPressedEvent));
	}
	bool ContentBrowserPanel::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		if (!m_ChildHovered && !m_PanelHovered) return false;

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

