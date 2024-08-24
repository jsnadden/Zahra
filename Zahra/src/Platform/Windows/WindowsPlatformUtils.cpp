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

	float Time::GetTime()
	{
		return glfwGetTime();
	}

	std::filesystem::path FileDialogs::OpenFile(const wchar_t* filterDescription, const wchar_t* filterExtension)
	{
		std::filesystem::path filepath;

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

		return filepath;
	}

	std::filesystem::path FileDialogs::SaveFile(const wchar_t* filterDescription, const wchar_t* filterExtension)
	{
		std::filesystem::path filepath;

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

		return filepath;
	}


}


