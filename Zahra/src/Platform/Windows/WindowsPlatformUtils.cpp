#include "zpch.h"
#include "Zahra/Utils/PlatformUtils.h"

#include "Zahra/Core/Application.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Zahra
{
	std::optional<std::string> FileDialogs::OpenFile(const char* filter)
	{
		// windows api for a file opening dialog (the A instructs windows to interpret the file as ascii encoded)
		OPENFILENAMEA openFileName;
		CHAR fileBuffer[260] = { 0 }; // buffer used for filepath
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&openFileName, sizeof(OPENFILENAME));
		openFileName.lStructSize = sizeof(OPENFILENAME);
		openFileName.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		openFileName.lpstrFile = fileBuffer;
		openFileName.nMaxFile = sizeof(fileBuffer);
		if (GetCurrentDirectoryA(256, currentDir)) openFileName.lpstrInitialDir = currentDir;
		openFileName.lpstrFilter = filter; // populates the "filter by file type" combo box
		openFileName.nFilterIndex = 1; // default file type to filter by
		openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; // last flag makes sure the working directory doesn't change to opened file location!!

		if (GetOpenFileNameA(&openFileName) == TRUE)
		{
			return openFileName.lpstrFile;
		}

		return std::nullopt;
	}

	std::optional<std::string> FileDialogs::SaveFile(const char* filter)
	{
		// see above for details
		OPENFILENAMEA openFileName;
		CHAR fileBuffer[260] = { 0 };
		ZeroMemory(&openFileName, sizeof(OPENFILENAME));
		openFileName.lStructSize = sizeof(OPENFILENAME);
		openFileName.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		openFileName.lpstrFile = fileBuffer;
		openFileName.nMaxFile = sizeof(fileBuffer);
		openFileName.lpstrFilter = filter;
		openFileName.nFilterIndex = 1;
		openFileName.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
		openFileName.lpstrDefExt = std::strchr(filter, '\0') + 1;// Sets the default extension by extracting it from the filter
		CHAR currentDir[256] = { 0 };
		if (GetCurrentDirectoryA(256, currentDir)) openFileName.lpstrInitialDir = currentDir;

		if (GetSaveFileNameA(&openFileName) == TRUE)
		{
			return openFileName.lpstrFile;
		}

		return std::nullopt;
	}


}


