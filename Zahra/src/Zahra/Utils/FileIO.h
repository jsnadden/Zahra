#pragma once

namespace Zahra
{
	class FileIO
	{
	public:
		static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize = nullptr);


	};
}
