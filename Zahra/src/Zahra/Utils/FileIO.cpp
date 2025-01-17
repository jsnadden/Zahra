#include "zpch.h"
#include "FileIO.h"

namespace Zahra
{
	char* FileIO::ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
	{
		std::ifstream fileStream(filepath, std::ios::binary | std::ios::ate);

		if (!fileStream)
		{
			// Failed to open the file
			return nullptr;
		}

		std::streampos end = fileStream.tellg();
		fileStream.seekg(0, std::ios::beg);
		uint32_t size = (uint32_t)(end - fileStream.tellg());

		if (size == 0)
		{
			// File is empty
			return nullptr;
		}

		char* buffer = znew char[size];
		fileStream.read((char*)buffer, size);
		fileStream.close();

		if (outSize) *outSize = size;

		return buffer;
	}

	std::string FileIO::ReadAsString(const std::filesystem::path& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			auto fileSize = in.tellg();
			const int skippedChars = SkipBOM(in);

			fileSize -= skippedChars - 1;
			result.resize(fileSize);
			in.read(result.data() + 1, fileSize);
			// Add a dummy tab to beginning of file.
			result[0] = '\t';
		}
		in.close();
		return result;
	}

	int FileIO::SkipBOM(std::istream& in)
	{
		char test[4] = { 0 };
		in.seekg(0, std::ios::beg);
		in.read(test, 3);
		if (strcmp(test, "\xEF\xBB\xBF") == 0)
		{
			in.seekg(3, std::ios::beg);
			return 3;
		}
		in.seekg(0, std::ios::beg);
		return 0;
	}

}

