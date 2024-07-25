#include "zpch.h"
#include "ContentBrowserPanel.h"

#include <ImGui/imgui.h>

// TODO: later we'll get rid of this being hardcoded, in favour of a "projects" system
static const std::filesystem::path s_AssetsRoot = "assets";

namespace Zahra
{
	
	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentPath(s_AssetsRoot)
	{

	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		// TODO: currently scanning directories per frame, maybe make this only refresh
		// once per second(?), caching the entries (and obvs keep displaying per frame)

		ImGui::Begin("ContentBrowserPanel");

		// Comment one out
		//RecurseFileTreeNodes(m_CurrentPath);
		NavigateDirs();

		ImGui::End();
	}

	
	void ContentBrowserPanel::RecurseFileTreeNodes(std::filesystem::path filepath)
	{
		for (auto& item : std::filesystem::directory_iterator(filepath))
		{
			const std::filesystem::path& path = item.path().filename();
			std::string pathString = path.string();

			if (item.is_directory())
			{
				if (ImGui::TreeNodeEx(pathString.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
				{
					RecurseFileTreeNodes(filepath / path);

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

	void ContentBrowserPanel::NavigateDirs()
	{
		if (m_CurrentPath != s_AssetsRoot)
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentPath = m_CurrentPath.parent_path();
			}
		}

		for (auto entry : std::filesystem::directory_iterator(m_CurrentPath))
		{
			const std::filesystem::path& path = entry.path();
			std::filesystem::path relativePath = std::filesystem::relative(path, s_AssetsRoot);
			std::string pathString = relativePath.filename().string();

			if (entry.is_directory())
			{
				if (ImGui::Button(pathString.c_str()))
				{
					m_CurrentPath /= path.filename();
				}
			}
			else
			{
				ImGui::Text(pathString.c_str());
			}

		}
	}

	


}

