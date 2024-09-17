#pragma once

namespace Zahra
{
	class FileIO
	{
	public:
		static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize = nullptr);

		static std::string ReadAsString(const std::filesystem::path& filepath, uint32_t* outSize = nullptr);

		static int SkipBOM(std::istream& in);
	};
}
