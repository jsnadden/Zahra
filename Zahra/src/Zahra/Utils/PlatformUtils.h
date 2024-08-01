#pragma once

#include <string>
#include <optional>

namespace Zahra
{
	class FileDialogs
	{
	public:
		static std::filesystem::path OpenFile(const wchar_t* filterDescription, const wchar_t* filterExtension);
		static std::filesystem::path SaveFile(const wchar_t* filterDescription, const wchar_t* filterExtension);


	};
}
