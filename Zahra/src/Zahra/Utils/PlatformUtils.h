#pragma once

#include <string>
#include <optional>

namespace Zahra
{
	struct FileTypeFilter
	{
		std::string Description;
		std::string Extension;
	};

	class FileDialogs
	{
	public:
		static std::filesystem::path OpenFile(FileTypeFilter filter);
		static std::filesystem::path SaveFile(FileTypeFilter filter);


	};

	class Time
	{
	public:
		static float GetTime();
	};
}
