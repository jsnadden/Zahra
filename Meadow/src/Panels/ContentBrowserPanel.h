#pragma once

#include <filesystem>


namespace Zahra
{

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		// TODO: choose one or the other (or something better?)
		// one approach is to show the whole directory substructure as treenodes
		void RecurseFileTreeNodes(std::filesystem::path filepath);
		// another approach is to just show one directory at a time, with navigation buttons button
		void NavigateDirs();

	private:
		std::filesystem::path m_CurrentPath;
	};

}
