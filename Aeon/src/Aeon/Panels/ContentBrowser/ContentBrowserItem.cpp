#include "ContentBrowserItem.h"
#include <CommonUtilities/StringUtils.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Editor/EditorSettings.h>
#include <Epoch/Editor/SelectionManager.h>
#include "Aeon/EditorResources.h"

namespace Epoch
{
	static char s_RenameBuffer[MAX_INPUT_BUFFER_LENGTH];

	//ContentBrowserItem
	ContentBrowserItem::ContentBrowserItem(ItemType aType, AssetHandle aId, const std::string& aName, const std::shared_ptr<Texture2D>& aIcon) :
		myType(aType), myID(aId), myFileName(aName), myIcon(aIcon)
	{
		myDisplayName = myFileName;
		if (myFileName.size() > 25)
		{
			myDisplayName = myFileName.substr(0, 25) + "...";
		}
	}

	void ContentBrowserItem::OnRenderBegin()
	{
		ImGui::PushID(&myID);
		ImGui::BeginGroup();
	}

	//TODO: Implement
	CBItemActionResult ContentBrowserItem::OnRender()
	{
		return CBItemActionResult();
	}

	void ContentBrowserItem::OnRenderEnd()
	{
		ImGui::PopID();
		ImGui::NextColumn();
	}

	void ContentBrowserItem::StartRenaming()
	{
		if (myIsRenaming)
		{
			return;
		}

		memset(s_RenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
		memcpy(s_RenameBuffer, myFileName.c_str(), myFileName.size());
		myIsRenaming = true;
	}

	void ContentBrowserItem::StopRenaming()
	{
		myIsRenaming = false;
		SetDisplayNameFromFileName();
		memset(s_RenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
	}

	void ContentBrowserItem::Rename(const std::string& aNewName)
	{
		OnRenamed(aNewName);
	}

	void ContentBrowserItem::SetDisplayNameFromFileName()
	{
		const auto& editorSettings = EditorSettings::Get();
		const float thumbnailSize = editorSettings.contentBrowserThumbnailSize;

		int maxCharacters = (int)(0.00152587f * (thumbnailSize * thumbnailSize)); // 0.00152587f is a magic number that is gained from graphing this equation in desmos and setting the y=25 at x=128

		if (myFileName.size() > maxCharacters)
		{
			myDisplayName = myFileName.substr(0, maxCharacters) + " ...";
		}
		else
		{
			myDisplayName = myFileName;
		}
	}

	void ContentBrowserItem::OnContextMenuOpen(CBItemActionResult& aActionResult)
	{
		if (ImGui::MenuItem("Reload"))
			aActionResult.Set(ContentBrowserAction::Reload, true);

		if (ImGui::MenuItem("Rename"), 0, false, SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) == 1)
			aActionResult.Set(ContentBrowserAction::StartRenaming, true);

		if (ImGui::MenuItem("Copy", "Ctrl+C"))
			aActionResult.Set(ContentBrowserAction::Copy, true);

		if (ImGui::MenuItem("Duplicate"))
			aActionResult.Set(ContentBrowserAction::Duplicate, true);

		if (ImGui::MenuItem("Delete", "Ctrl+D"))
			aActionResult.Set(ContentBrowserAction::OpenDeleteDialogue, true);

		ImGui::Separator();

		if (ImGui::MenuItem("Show In Explorer"))
			aActionResult.Set(ContentBrowserAction::ShowInExplorer, true);

		if (ImGui::MenuItem("Open Externally"))
			aActionResult.Set(ContentBrowserAction::OpenExternal, true);
	}

	//ContentBrowserDirectory
	ContentBrowserDirectory::ContentBrowserDirectory(const std::shared_ptr<DirectoryInfo>& aDirectoryInfo) :
		ContentBrowserItem(ContentBrowserItem::ItemType::Directory, aDirectoryInfo->handle, aDirectoryInfo->filePath.filename().string(), EditorResources::DirectoryIcon), myDirectoryInfo(aDirectoryInfo) {}

	void ContentBrowserDirectory::Delete()
	{
		bool deleted = FileSystem::DeleteFile(Project::GetAssetDirectory() / myDirectoryInfo->filePath);
		if (!deleted)
		{
			LOG_ERROR("Failed to delete folder {}", myDirectoryInfo->filePath);
			return;
		}

		for (auto asset : myDirectoryInfo->assets)
		{
			Project::GetEditorAssetManager()->OnAssetDeleted(asset);
		}
	}

	bool ContentBrowserDirectory::Move(const std::filesystem::path& aDestination)
	{
		bool wasMoved = FileSystem::MoveFile(Project::GetAssetDirectory() / myDirectoryInfo->filePath, Project::GetAssetDirectory() / aDestination);
		if (!wasMoved)
		{
			return false;
		}

		return true;
	}

	void ContentBrowserDirectory::OnRenamed(const std::string& aNewName)
	{
		auto target = Project::GetAssetDirectory() / myDirectoryInfo->filePath;
		auto destination = Project::GetAssetDirectory() / myDirectoryInfo->filePath.parent_path() / aNewName;

		if (CU::ToLower(aNewName) == CU::ToLower(target.filename().string()))
		{
			auto tmp = Project::GetAssetDirectory() / myDirectoryInfo->filePath.parent_path() / "TempDir";
			FileSystem::Rename(target, tmp);
			target = tmp;
		}

		if (!FileSystem::Rename(target, destination))
		{
			LOG_ERROR("Couldn't rename {} to {}!", myDirectoryInfo->filePath.filename().string(), aNewName);
		}
	}

	//TODO: Implement
	void ContentBrowserDirectory::UpdateDrop(CBItemActionResult& aActionResult)
	{
		//if (SelectionManager::IsSelected(SelectionContext::Asset, m_ID))
		//{
		//	return;
		//}
		//
		//if (ImGui::BeginDragDropTarget())
		//{
		//	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset_payload");
		//
		//	if (payload)
		//	{
		//		auto& currentItems = ContentBrowserPanel::Get().GetCurrentItems();
		//		uint32_t count = payload->DataSize / sizeof(AssetHandle);
		//
		//		for (uint32_t i = 0; i < count; i++)
		//		{
		//			AssetHandle assetHandle = *(((AssetHandle*)payload->Data) + i);
		//			size_t index = currentItems.FindItem(assetHandle);
		//			if (index != ContentBrowserItemList::InvalidItem)
		//			{
		//				if (currentItems[index]->Move(m_DirectoryInfo->FilePath))
		//				{
		//					aActionResult.Set(ContentBrowserAction::Refresh, true);
		//					currentItems.erase(assetHandle);
		//				}
		//			}
		//		}
		//	}
		//
		//	ImGui::EndDragDropTarget();
		//}
	}

	//ContentBrowserAsset
	ContentBrowserAsset::ContentBrowserAsset(const AssetMetadata& aAssetInfo, const std::shared_ptr<Texture2D>& aIcon) :
		ContentBrowserItem(ContentBrowserItem::ItemType::Asset, aAssetInfo.handle, aAssetInfo.filePath.stem().string(), aIcon), myAssetInfo(aAssetInfo) {}

	//TODO: Implement
	void ContentBrowserAsset::Delete()
	{
		//auto filepath = Project::GetEditorAssetManager()->GetFileSystemPath(m_AssetInfo);
		//bool deleted = FileSystem::DeleteFile(filepath);
		//if (!deleted)
		//{
		//	LOG_ERROR("Couldn't delete {}", m_AssetInfo.filePath);
		//	return;
		//}
		//
		//auto currentDirectory = ContentBrowserPanel::Get().GetDirectory(m_AssetInfo.filePath.parent_path());
		//currentDirectory->Assets.erase(std::remove(currentDirectory->Assets.begin(), currentDirectory->Assets.end(), m_AssetInfo.handle), currentDirectory->Assets.end());
		//
		//Project::GetEditorAssetManager()->OnAssetDeleted(m_AssetInfo.handle);
	}

	bool ContentBrowserAsset::Move(const std::filesystem::path& aDestination)
	{
		auto filepath = Project::GetEditorAssetManager()->GetFileSystemPath(myAssetInfo);
		bool wasMoved = FileSystem::MoveFile(filepath, Project::GetAssetDirectory() / aDestination);
		if (!wasMoved)
		{
			LOG_ERROR("Couldn't move {} to {}", myAssetInfo.filePath, aDestination.string());
			return false;
		}

		Project::GetEditorAssetManager()->OnAssetRenamed(myAssetInfo.handle, aDestination / filepath.filename());
		return true;
	}

	void ContentBrowserAsset::OnRenamed(const std::string& aNewName)
	{
		auto filepath = Project::GetEditorAssetManager()->GetFileSystemPath(myAssetInfo);
		const std::string extension = filepath.extension().string();
		std::filesystem::path newFilepath = fmt::format("{0}\\{1}{2}", filepath.parent_path().string(), aNewName, extension);

		std::string targetName = fmt::format("{0}{1}", aNewName, extension);
		if (CU::ToLower(targetName) == CU::ToLower(filepath.filename().string()))
		{
			FileSystem::RenameFilename(filepath, "temp-rename");
			filepath = fmt::format("{0}\\temp-rename{1}", filepath.parent_path().string(), extension);
		}

		if (FileSystem::RenameFilename(filepath, aNewName))
		{
			// Update AssetManager with new name
			Project::GetEditorAssetManager()->OnAssetRenamed(myAssetInfo.handle, newFilepath);
		}
		else
		{
			LOG_ERROR("Couldn't rename {} to {}!", filepath.filename().string(), aNewName);
		}
	}
}
