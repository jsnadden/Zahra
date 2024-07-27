#pragma once

#include <string>
#include <optional>

namespace Zahra
{
	class FileDialogs
	{
	public:
		// TODO: refactor these to return std::filesystem::path instead!!
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);


	};
}
