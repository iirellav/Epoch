#pragma once
#include <filesystem>
#include "Epoch/Core/Buffer.h"

#ifdef CreateDirectory
#undef CreateDirectory
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef SetEnvironmentVariable
#undef GetEnvironmentVariable
#endif

namespace Epoch
{
	enum class FilewatchEvent { Added, Removed, Modified, RenamedOld, RenamedNew };

	class FileSystem
	{
	public:
		static bool CreateDirectory(const std::filesystem::path& aDirectory);
		static bool IsDirectory(const std::filesystem::path& aFilepath);
		static bool Exists(const std::filesystem::path& aFilepath);
		static bool DeleteFile(const std::filesystem::path& aFilepath);
		static void DeleteContent(const std::filesystem::path& aFilepath);
		static bool MoveFile(const std::filesystem::path& aFilepath, const std::filesystem::path& aDest);
		static bool CopyFile(const std::filesystem::path& aFilepath, const std::filesystem::path& aDest);

		static bool IsNewer(const std::filesystem::path& aFileA, const std::filesystem::path& aFileB);

		static bool Move(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath);
		static bool Copy(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath);
		static bool CopyContent(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath);
		static bool Rename(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath);
		static bool RenameFilename(const std::filesystem::path& aOldFilepath, const std::string& aNewName);

		static bool ShowFileInExplorer(const std::filesystem::path& aPath);
		static bool OpenDirectoryInExplorer(const std::filesystem::path& aPath);
		static bool OpenExternally(const std::filesystem::path& aPath);
		
		static bool WriteBytes(const std::filesystem::path& aFilepath, const Buffer& aBuffer);
		static Buffer ReadBytes(const std::filesystem::path& aFilepath);

		static std::filesystem::path GetUniqueFileName(const std::filesystem::path& aFilepath);

		struct FileDialogFilterItem
		{
			const char* name;
			const char* spec;
		};

		static std::filesystem::path OpenFileDialog(const std::initializer_list<FileDialogFilterItem> aFilters = {}, const char* aInitialFolder = "");
		static std::vector<std::filesystem::path> OpenFileDialogMultiple(const std::initializer_list<FileDialogFilterItem> aFilters = {}, const char* aInitialFolder = "");
		static std::filesystem::path OpenFolderDialog(const char* aInitialFolder = "");
		static std::filesystem::path SaveFileDialog(const std::initializer_list<FileDialogFilterItem> aFilters = {}, const char* aInitialFolder = "");

		
		static bool HasEnvironmentVariable(const std::string& aKey);
		static bool SetEnvironmentVariable(const std::string& aKey, const std::string& aValue);
		static std::string GetEnvironmentVariable(const std::string& aKey);
	};
}
