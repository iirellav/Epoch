#include "ContentBrowserItem.h"
#include <CommonUtilities/StringUtils.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Core/Input.h>
#include <Epoch/Editor/EditorSettings.h>
#include <Epoch/Editor/SelectionManager.h>
#include "Editor/EditorResources.h"
#include "Editor/Panels/ContentBrowserPanel.h"

namespace Epoch
{
	static char staticRenameBuffer[MAX_INPUT_BUFFER_LENGTH];

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

	CBItemActionResult ContentBrowserItem::OnRender()
	{
		CBItemActionResult result;

		const float thumbnailSize = EditorSettings::Get().contentBrowserThumbnailSize;

		SetDisplayNameFromFileName();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		const float edgeOffset = 4.0f;

		const float textLineHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0f + edgeOffset * 2.0f;
		const float infoPanelHeight = textLineHeight;

		const ImVec2 topLeft = ImGui::GetCursorScreenPos();
		const ImVec2 thumbBottomRight = { topLeft.x + thumbnailSize, topLeft.y + thumbnailSize };
		const ImVec2 infoTopLeft =		{ topLeft.x,				 topLeft.y + thumbnailSize };
		const ImVec2 bottomRight =		{ topLeft.x + thumbnailSize, topLeft.y + thumbnailSize + infoPanelHeight };

		const bool isFocused = ImGui::IsWindowFocused();

		const bool isSelected = SelectionManager::IsSelected(SelectionContext::ContentBrowser, myID);

		// Thumbnail
		auto icon = myIcon ? myIcon : EditorResources::OtherIcon;
		ImGui::InvisibleButton("##thumbnailButton", ImVec2{ thumbnailSize, thumbnailSize });
		UI::Draw::DrawButtonImage(icon,
			IM_COL32(255, 255, 255, 225),
			IM_COL32(255, 255, 255, 255),
			IM_COL32(255, 255, 255, 255),
			UI::RectExpanded(UI::GetItemRect(), -6.0f, -6.0f));

		auto renamingWidget = [&]()
			{
				ImGui::SetKeyboardFocusHere();
				ImGui::InputText("##rename", staticRenameBuffer, MAX_INPUT_BUFFER_LENGTH);

				if (ImGui::IsItemDeactivatedAfterEdit() || Input::IsKeyPressed(KeyCode::Enter))
				{
					Rename(staticRenameBuffer);
					myIsRenaming = false;
					SetDisplayNameFromFileName();
					result.Set(ContentBrowserAction::Renamed, true);
				}
				else if (Input::IsKeyPressed(KeyCode::Escape))
				{
					myIsRenaming = false;
				}
			};

		const float textWidth = std::min(ImGui::CalcTextSize(myDisplayName.c_str()).x, thumbnailSize);
		{
			if (myIsRenaming)
			{
				//UI::ShiftCursorX(thumbnailSize * 0.5f - textWidth * 0.5f);
				ImGui::SetNextItemWidth(thumbnailSize);
				renamingWidget();
			}
			else
			{
				const bool isSelected = SelectionManager::IsSelected(SelectionContext::ContentBrowser, myID);
				UI::ShiftCursorX(thumbnailSize * 0.5f - textWidth * 0.5f);
				ImGui::SetNextItemWidth(textWidth);

				if (isSelected) ImGui::PushStyleColor(ImGuiCol_Text, Colors::Theme::blue);
				ImGui::Text(myDisplayName.c_str());
				if (isSelected) ImGui::PopStyleColor();

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					result.Set(ContentBrowserAction::Hovered, true);
					result.Set(ContentBrowserAction::ClearSelections, true);
					result.Set(ContentBrowserAction::Selected, true);
					result.Set(ContentBrowserAction::StartRenaming, true);
				}
			}
		}

		if (!myIsRenaming)
		{
			if (Input::IsKeyPressed(KeyCode::F2) && isSelected && isFocused)
			{
				result.Set(ContentBrowserAction::Hovered, true);
				result.Set(ContentBrowserAction::ClearSelections, true);
				result.Set(ContentBrowserAction::Selected, true);
				result.Set(ContentBrowserAction::StartRenaming, true);
			}
		}

		ImGui::PopStyleVar(); // ItemSpacing

		// End of the Item Group
		ImGui::EndGroup();

		UpdateDrop(result);

		bool dragging = false;
		if (dragging = ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID | ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
		{
			myIsDragging = true;

			const auto& selectionStack = SelectionManager::GetSelections(SelectionContext::ContentBrowser);
			if (!SelectionManager::IsSelected(SelectionContext::ContentBrowser, myID))
			{
				result.Set(ContentBrowserAction::ClearSelections, true);
			}

			auto& currentItems = ContentBrowserPanel::Get().GetCurrentItems();

			if (!selectionStack.empty())
			{
				for (const auto& selectedItemHandles : selectionStack)
				{
					size_t index = currentItems.FindItem(selectedItemHandles);
					if (index == ContentBrowserItemList::InvalidItem)
					{
						continue;
					}

					const auto& item = currentItems[index];
					UI::Draw::Image(item->GetIcon(), ImVec2(18, 18));
					ImGui::SameLine();
					const auto& name = item->GetName();
					ImGui::TextUnformatted(name.c_str());
				}

				ImGui::SetDragDropPayload("asset_payload", selectionStack.data(), sizeof(AssetHandle) * selectionStack.size());
			}

			result.Set(ContentBrowserAction::Selected, true);
			ImGui::EndDragDropSource();
		}

		if (ImGui::IsItemHovered())
		{
			result.Set(ContentBrowserAction::Hovered, true);

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !myIsRenaming && !result.IsSet(ContentBrowserAction::StartRenaming))
			{
				result.Set(ContentBrowserAction::Activated, true);
			}
			else
			{
				bool leftClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
				bool rightClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Right);

				const bool isSelected = SelectionManager::IsSelected(SelectionContext::ContentBrowser, myID);
				bool skipBecauseDragging = myIsDragging && isSelected;

				if ((leftClicked || rightClicked) && !skipBecauseDragging)
				{
					if (myJustSelected)
					{
						myJustSelected = false;
					}

					if (isSelected && Input::IsKeyHeld(KeyCode::LeftControl) && !myJustSelected)
					{
						result.Set(ContentBrowserAction::Deselected, true);
					}

					if (!isSelected)
					{
						result.Set(ContentBrowserAction::Selected, true);
						myJustSelected = true;
					}

					if (!Input::IsKeyHeld(KeyCode::LeftControl) && !Input::IsKeyHeld(KeyCode::LeftShift) && myJustSelected)
					{
						result.Set(ContentBrowserAction::ClearSelections, true);
					}

					if (Input::IsKeyHeld(KeyCode::LeftShift))
					{
						result.Set(ContentBrowserAction::SelectToHere, true);
					}
				}
			}
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
		if (ImGui::BeginPopupContextItem("CBItemContextMenu"))
		{
			result.Set(ContentBrowserAction::Selected, true);

			OnContextMenuOpen(result);

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		myIsDragging = dragging;

		return result;
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

		memset(staticRenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
		memcpy(staticRenameBuffer, myFileName.c_str(), myFileName.size());
		myIsRenaming = true;
	}

	void ContentBrowserItem::StopRenaming()
	{
		myIsRenaming = false;
		SetDisplayNameFromFileName();
		memset(staticRenameBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
	}

	void ContentBrowserItem::Rename(const std::string& aNewName)
	{
		OnRenamed(aNewName);
	}

	void ContentBrowserItem::SetDisplayNameFromFileName()
	{
		const float thumbnailSize = EditorSettings::Get().contentBrowserThumbnailSize * 0.9f;

		int maxCharacters = (int)(0.00152587f * (thumbnailSize * thumbnailSize)); // 0.00152587f is a magic number that is gained from graphing this equation in desmos and setting the y=25 at x=128

		if (myFileName.size() > maxCharacters)
		{
			myDisplayName = myFileName.substr(0, maxCharacters - 3) + "...";
		}
		else
		{
			myDisplayName = myFileName;
		}
	}

	void ContentBrowserItem::OnContextMenuOpen(CBItemActionResult& aActionResult)
	{
		if (ImGui::MenuItem("Reload"))
		{
			aActionResult.Set(ContentBrowserAction::Reload, true);
		}

		if (ImGui::MenuItem("Rename", 0, false, SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) == 1))
		{
			aActionResult.Set(ContentBrowserAction::StartRenaming, true);
		}

		if (ImGui::MenuItem("Copy", "Ctrl+C"))
		{
			aActionResult.Set(ContentBrowserAction::Copy, true);
		}

		if (ImGui::MenuItem("Duplicate"))
		{
			aActionResult.Set(ContentBrowserAction::Duplicate, true);
		}

		if (ImGui::MenuItem("Delete", "Ctrl+D"))
		{
			aActionResult.Set(ContentBrowserAction::OpenDeleteDialogue, true);
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Show In Explorer"))
		{
			aActionResult.Set(ContentBrowserAction::ShowInExplorer, true);
		}

		if (ImGui::MenuItem("Open Externally"))
		{
			aActionResult.Set(ContentBrowserAction::OpenExternal, true);
		}
	}

	//ContentBrowserDirectory
	ContentBrowserDirectory::ContentBrowserDirectory(const std::shared_ptr<DirectoryInfo>& aDirectoryInfo) :
		ContentBrowserItem(ContentBrowserItem::ItemType::Directory, aDirectoryInfo->handle, aDirectoryInfo->filePath.filename().string(), EditorResources::ClosedFolderIcon), myDirectoryInfo(aDirectoryInfo) {}

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

	void ContentBrowserDirectory::UpdateDrop(CBItemActionResult& aActionResult)
	{
		if (SelectionManager::IsSelected(SelectionContext::ContentBrowser, myID))
		{
			return;
		}
		
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset_payload");
		
			if (payload)
			{
				auto& currentItems = ContentBrowserPanel::Get().GetCurrentItems();
				uint32_t count = payload->DataSize / sizeof(AssetHandle);
		
				for (uint32_t i = 0; i < count; i++)
				{
					AssetHandle assetHandle = *(((AssetHandle*)payload->Data) + i);
					size_t index = currentItems.FindItem(assetHandle);
					if (index != ContentBrowserItemList::InvalidItem)
					{
						if (currentItems[index]->Move(myDirectoryInfo->filePath))
						{
							aActionResult.Set(ContentBrowserAction::Refresh, true);
							currentItems.Erase(assetHandle);
						}
					}
				}
			}
		
			ImGui::EndDragDropTarget();
		}
	}

	//ContentBrowserAsset
	ContentBrowserAsset::ContentBrowserAsset(const AssetMetadata& aAssetInfo, const std::shared_ptr<Texture2D>& aIcon) :
		ContentBrowserItem(ContentBrowserItem::ItemType::Asset, aAssetInfo.handle, aAssetInfo.filePath.stem().string(), aIcon), myAssetInfo(aAssetInfo) {}

	void ContentBrowserAsset::Delete()
	{
		auto filepath = Project::GetEditorAssetManager()->GetFileSystemPath(myAssetInfo);
		bool deleted = FileSystem::DeleteFile(filepath);
		if (!deleted)
		{
			LOG_ERROR("Couldn't delete {}", myAssetInfo.filePath);
			return;
		}
		
		auto currentDirectory = ContentBrowserPanel::Get().GetDirectory(myAssetInfo.filePath.parent_path());
		currentDirectory->assets.erase(std::remove(currentDirectory->assets.begin(), currentDirectory->assets.end(), myAssetInfo.handle), currentDirectory->assets.end());
		
		Project::GetEditorAssetManager()->OnAssetDeleted(myAssetInfo.handle);
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
