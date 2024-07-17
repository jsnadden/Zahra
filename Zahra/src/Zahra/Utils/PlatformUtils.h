#pragma once

#include <string>
#include <optional>

namespace Zahra
{
	class FileDialogs
	{
	public:
		// these return an empty string if cancelled!
		static std::optional<std::string> OpenFile(const char* filter);
		static std::optional<std::string> SaveFile(const char* filter);


	};
}
