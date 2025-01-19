#include "zpch.h"
#include "Zahra/Utils/PlatformUtils.h"

#include "Zahra/Core/Application.h"

#include <commdlg.h>
#include <shobjidl.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Zahra
{
	// NOTE: calls new, so need to manually free the output after use
	static const wchar_t* ConvertStringToWideChar(const std::string& string)
	{
		const size_t newSize = string.size() + 1;
		wchar_t* convertedString = znew wchar_t[newSize];
		size_t conversionCount = 0;
		mbstowcs_s(&conversionCount, convertedString, newSize, string.c_str(), _TRUNCATE);

		return convertedString;
	}

	float Time::GetTime()
	{
		return (float)glfwGetTime();
	}

	std::filesystem::path FileDialogs::OpenFile(FileTypeFilter filter)
	{
		std::filesystem::path filepath;

		const wchar_t* filterDescription = ConvertStringToWideChar(filter.Description);
		const wchar_t* filterExtension = ConvertStringToWideChar(filter.Extension);

		IFileOpenDialog* openDialog;
		HRESULT hresult = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&openDialog));
		Z_CORE_ASSERT(SUCCEEDED(hresult), "Failed to create IFileOpenDialog instance");

		// set default directory to cwd
		{
			std::filesystem::path currentDirectory = std::filesystem::current_path();

			IShellItem* currentDirItem;
			hresult = SHCreateItemFromParsingName(currentDirectory.wstring().c_str(), NULL, IID_PPV_ARGS(&currentDirItem));
			
			if (!SUCCEEDED(hresult))
				Z_CORE_WARN("SHCreateItemFromParsingName failed to parse current directory path");
			else
				openDialog->SetDefaultFolder(currentDirItem);

			currentDirItem->Release();
		}

		// set filetype filter and default extension
		{
			COMDLG_FILTERSPEC typeFilter[2] = { { filterDescription, filterExtension }, { L"All files", L"*.*"}};
			openDialog->SetFileTypes(2, typeFilter);
			openDialog->SetFileTypeIndex(0);

			/*std::wstring defaultExtension = filterExtension;
			defaultExtension = defaultExtension.substr(defaultExtension.find_first_of(L".") + 1);
			openDialog->SetDefaultExtension(defaultExtension.c_str());*/
		}

		// Show the Open dialog box.
		hresult = openDialog->Show(NULL);
		if (SUCCEEDED(hresult))

		{
			IShellItem* selectedFileItem;
			openDialog->GetResult(&selectedFileItem);

			PWSTR filepathString;
			selectedFileItem->GetDisplayName(SIGDN_FILESYSPATH, &filepathString);

			filepath = filepathString;

			CoTaskMemFree(filepathString);
			selectedFileItem->Release();
		}

		openDialog->Release();

		delete[] filterDescription;
		delete[] filterExtension;

		return filepath;
	}

	std::filesystem::path FileDialogs::SaveFile(FileTypeFilter filter)
	{
		std::filesystem::path filepath;

		const wchar_t* filterDescription = ConvertStringToWideChar(filter.Description);
		const wchar_t* filterExtension = ConvertStringToWideChar(filter.Extension);

		IFileSaveDialog* saveDialog;
		HRESULT hresult = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			IID_IFileSaveDialog, reinterpret_cast<void**>(&saveDialog));
		Z_CORE_ASSERT(SUCCEEDED(hresult), "Failed to create IFileSaveDialog instance");

		// set default directory to cwd
		{
			std::filesystem::path currentDirectory = std::filesystem::current_path();

			IShellItem* currentDirItem;
			hresult = SHCreateItemFromParsingName(currentDirectory.wstring().c_str(), NULL, IID_PPV_ARGS(&currentDirItem));

			if (!SUCCEEDED(hresult))
				Z_CORE_WARN("SHCreateItemFromParsingName failed to parse current directory path");
			else
				saveDialog->SetDefaultFolder(currentDirItem);

			currentDirItem->Release();
		}

		// set filetype filter and default extension
		{
			COMDLG_FILTERSPEC typeFilter[2] = { { filterDescription, filterExtension }, { L"All files", L"*.*"} };
			saveDialog->SetFileTypes(2, typeFilter);
			saveDialog->SetFileTypeIndex(0);

			std::wstring defaultExtension = filterExtension;
			defaultExtension = defaultExtension.substr(defaultExtension.find_first_of(L".") + 1);
			saveDialog->SetDefaultExtension(defaultExtension.c_str());
		}

		// Show the Open dialog box.
		hresult = saveDialog->Show(NULL);
		if (SUCCEEDED(hresult))

		{
			IShellItem* selectedFileItem;
			saveDialog->GetResult(&selectedFileItem);

			PWSTR filepathString;
			selectedFileItem->GetDisplayName(SIGDN_FILESYSPATH, &filepathString);

			filepath = filepathString;

			CoTaskMemFree(filepathString);
			selectedFileItem->Release();
		}

		saveDialog->Release();

		delete[] filterDescription;
		delete[] filterExtension;

		return filepath;
	}


}


