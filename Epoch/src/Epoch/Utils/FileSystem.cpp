#include "epch.h"
#include "FileSystem.h"
#include <nfd.hpp>

//#ifdef CreateDirectory
//#undef CreateDirectory
//#undef DeleteFile
//#undef MoveFile
//#undef CopyFile
//#undef SetEnvironmentVariable
//#undef GetEnvironmentVariable
//#endif

namespace Epoch
{
	bool FileSystem::CreateDirectory(const std::filesystem::path& aDirectory)
	{
		return std::filesystem::create_directories(aDirectory);
	}

	bool FileSystem::IsDirectory(const std::filesystem::path& aFilepath)
	{
		return std::filesystem::is_directory(aFilepath);
	}

	bool FileSystem::Exists(const std::filesystem::path& aFilepath)
	{
		return std::filesystem::exists(aFilepath);
	}

	bool FileSystem::DeleteFile(const std::filesystem::path& aFilepath)
	{
		if (!FileSystem::Exists(aFilepath)) return false;

		if (std::filesystem::is_directory(aFilepath))
		{
			return std::filesystem::remove_all(aFilepath) > 0;
		}
		return std::filesystem::remove(aFilepath);
	}

	void FileSystem::DeleteContent(const std::filesystem::path& aFilepath)
	{
		for (const auto& entry : std::filesystem::directory_iterator(aFilepath)) 
		{
			std::filesystem::remove_all(entry.path());
		}
	}

	bool FileSystem::MoveFile(const std::filesystem::path& aFilepath, const std::filesystem::path& aDest)
	{
		return Move(aFilepath, aDest / aFilepath.filename());
	}

	bool FileSystem::CopyFile(const std::filesystem::path& aFilepath, const std::filesystem::path& aDest)
	{
		return Copy(aFilepath, aDest / aFilepath.filename());
	}

	bool FileSystem::IsNewer(const std::filesystem::path& aFileA, const std::filesystem::path& aFileB)
	{
		return std::filesystem::last_write_time(aFileA) > std::filesystem::last_write_time(aFileB);
	}

	bool FileSystem::Move(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath)
	{
		if (FileSystem::Exists(aNewFilepath)) return false;

		std::filesystem::rename(aOldFilepath, aNewFilepath);
		return true;
	}

	bool FileSystem::Copy(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath, std::filesystem::copy_options aCopyOptions)
	{
		if (FileSystem::Exists(aNewFilepath)) return false;

		std::filesystem::copy(aOldFilepath, aNewFilepath, aCopyOptions);
		return true;
	}

	bool FileSystem::CopyContent(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath)
	{
		std::filesystem::copy(aOldFilepath, aNewFilepath, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

		return false;
	}

	bool FileSystem::Rename(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath)
	{
		return Move(aOldFilepath, aNewFilepath);
	}

	bool FileSystem::RenameFilename(const std::filesystem::path& aOldFilepath, const std::string& aNewName)
	{
		std::filesystem::path newPath = aOldFilepath.parent_path() / std::filesystem::path(aNewName + aOldFilepath.extension().string());
		return Rename(aOldFilepath, newPath);
	}

	bool FileSystem::ShowFileInExplorer(const std::filesystem::path& aPath)
	{
		auto absolutePath = std::filesystem::canonical(aPath);
		if (!FileSystem::Exists(absolutePath)) return false;

		std::string cmd = "explorer.exe /select,\"" + absolutePath.string() + "\"";

		system(cmd.c_str());
		return true;
	}

	bool FileSystem::OpenDirectoryInExplorer(const std::filesystem::path& aPath)
	{
		auto absolutePath = std::filesystem::canonical(aPath);
		if (!FileSystem::Exists(absolutePath)) return false;

		ShellExecute(NULL, L"explore", absolutePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		return true;
	}

	bool FileSystem::OpenExternally(const std::filesystem::path& aPath)
	{
		auto absolutePath = std::filesystem::canonical(aPath);
		if (!FileSystem::Exists(absolutePath)) return false;

		ShellExecute(NULL, L"open", absolutePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		return true;
	}

	bool FileSystem::WriteBytes(const std::filesystem::path& aFilepath, const Buffer& aBuffer)
	{
		std::ofstream stream(aFilepath, std::ios::binary | std::ios::trunc);

		if (!stream)
		{
			stream.close();
			return false;
		}

		stream.write((char*)aBuffer.data, aBuffer.size);
		stream.close();

		return true;
	}

	Buffer FileSystem::ReadBytes(const std::filesystem::path& aFilepath)
	{
		Buffer buffer;

		std::ifstream stream(aFilepath, std::ios::binary | std::ios::ate);
		EPOCH_ASSERT(stream, "Failed to open stream!");

		auto end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		auto size = end - stream.tellg();
		if (size == 0)
		{
			LOG_WARNING("File empty while trying to read bytes");
			return Buffer();
		}

		buffer.Allocate((uint32_t)size);
		stream.read((char*)buffer.data, buffer.size);
		stream.close();

		return buffer;
	}

	std::filesystem::path FileSystem::GetUniqueFileName(const std::filesystem::path& aFilepath)
	{
		if (!FileSystem::Exists(aFilepath))
		{
			return aFilepath;
		}

		int counter = 0;
		auto checkID = [&counter, aFilepath](auto checkID) -> std::filesystem::path
			{
				++counter;
				const std::string counterStr = [&counter]
				{
					if (counter < 10)
					{
						return "0" + std::to_string(counter);
					}
					else
					{
						return std::to_string(counter);
					}
				}();  // Pad with 0 if < 10;

				std::string newFileName = fmt::format("{} ({})", CU::RemoveExtension(aFilepath.filename().string()), counterStr);

				if (aFilepath.has_extension())
				{
					newFileName = fmt::format("{}{}", newFileName, aFilepath.extension().string());
				}

				if (FileSystem::Exists(aFilepath.parent_path() / newFileName))
				{
					return checkID(checkID);
				}
				else
				{
					return aFilepath.parent_path() / newFileName;
				}
			};

		return checkID(checkID);
	}

	std::filesystem::path FileSystem::OpenFileDialog(const std::initializer_list<FileDialogFilterItem> aFilters, const char* aInitialFolder)
	{
		NFD::UniquePath filePath;
		nfdresult_t result = NFD::OpenDialog(filePath, (const nfdfilteritem_t*)aFilters.begin(), (nfdfiltersize_t)aFilters.size(), (const nfdchar_t*)aInitialFolder);

		switch (result)
		{
		case NFD_OKAY: return filePath.get();
		case NFD_CANCEL: return "";
		case NFD_ERROR:
		{
			LOG_ERROR("NFD-Extended threw an error: {}", NFD::GetError());
			return "";
		}
		}

		return "";
	}

	std::vector<std::filesystem::path> FileSystem::OpenFileDialogMultiple(const std::initializer_list<FileDialogFilterItem> aFilters, const char* aInitialFolder)
	{
		NFD::UniquePathSet filePaths;
		nfdresult_t result = NFD::OpenDialogMultiple(filePaths, (const nfdfilteritem_t*)aFilters.begin(), (nfdfiltersize_t)aFilters.size(), (const nfdchar_t*)aInitialFolder);

		switch (result)
		{
		case NFD_OKAY:
		{
			nfdpathsetsize_t numPaths;
			NFD::PathSet::Count(filePaths, numPaths);

			std::vector<std::filesystem::path> outPaths(numPaths);
			for (nfdpathsetsize_t i = 0; i < numPaths; ++i)
			{
				NFD::UniquePathSetPath path;
				NFD::PathSet::GetPath(filePaths, i, path);
				outPaths.push_back(path.get());
			}

			return outPaths;
		}
		case NFD_CANCEL: return std::vector<std::filesystem::path>();
		case NFD_ERROR:
		{
			LOG_ERROR("NFD-Extended threw an error: {}", NFD::GetError());
			return std::vector<std::filesystem::path>();
		}
		}

		return std::vector<std::filesystem::path>();
	}

	std::filesystem::path FileSystem::OpenFolderDialog(const char* aInitialFolder)
	{
		NFD::UniquePath filePath;
		nfdresult_t result = NFD::PickFolder(filePath, (const nfdchar_t*)aInitialFolder);

		switch (result)
		{
		case NFD_OKAY: return filePath.get();
		case NFD_CANCEL: return "";
		case NFD_ERROR:
		{
			LOG_ERROR("NFD-Extended threw an error: {}", NFD::GetError());
			return "";
		}
		}

		return "";
	}

	std::filesystem::path FileSystem::SaveFileDialog(const std::initializer_list<FileDialogFilterItem> aFilters, const char* aInitialFolder)
	{
		NFD::UniquePath filePath;
		nfdresult_t result = NFD::SaveDialog(filePath, (const nfdfilteritem_t*)aFilters.begin(), (nfdfiltersize_t)aFilters.size(), (const nfdchar_t*)aInitialFolder);

		switch (result)
		{
		case NFD_OKAY: return filePath.get();
		case NFD_CANCEL: return "";
		case NFD_ERROR:
		{
			LOG_ERROR("NFD-Extended threw an error: {}", NFD::GetError());
			return "";
		}
		}

		return "";
	}


	bool FileSystem::HasEnvironmentVariable(const std::string& aKey)
	{
		HKEY hKey;
		LSTATUS lOpenStatus = RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_ALL_ACCESS, &hKey);

		if (lOpenStatus == ERROR_SUCCESS)
		{
			lOpenStatus = RegQueryValueExA(hKey, aKey.c_str(), 0, NULL, NULL, NULL);
			RegCloseKey(hKey);
		}

		return lOpenStatus == ERROR_SUCCESS;
	}

	bool FileSystem::SetEnvironmentVariable(const std::string& aKey, const std::string& aValue)
	{
		HKEY hKey;
		LPCSTR keyPath = "Environment";
		DWORD createdNewKey;
		LSTATUS lOpenStatus = RegCreateKeyExA(HKEY_CURRENT_USER, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &createdNewKey);
		if (lOpenStatus == ERROR_SUCCESS)
		{
			LSTATUS lSetStatus = RegSetValueExA(hKey, aKey.c_str(), 0, REG_SZ, (LPBYTE)aValue.c_str(), (DWORD)aValue.length() + 1);
			RegCloseKey(hKey);

			if (lSetStatus == ERROR_SUCCESS)
			{
				SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_BLOCK, 100, NULL);
				return true;
			}
		}

		return false;
	}

	std::string FileSystem::GetEnvironmentVariable(const std::string& aKey)
	{
		const char* value = getenv(aKey.c_str());
		if (value)
		{
			return std::string(value);
		}
		else
		{
			return {};
		}
	}
}
