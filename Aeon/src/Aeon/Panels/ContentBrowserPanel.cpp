#include "ContentBrowserPanel.h"
#include <fstream>
#include <CommonUtilities/Math/CommonMath.hpp>
#include <CommonUtilities/StringUtils.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Core/Input.h>
#include <Epoch/Debug/Log.h>
#include <Epoch/Debug/Instrumentor.h>
#include <Epoch/Utils/FileSystem.h>
#include <Epoch/Project/Project.h>
#include <Epoch/Scene/Scene.h>
#include <Epoch/Scene/Prefab.h>
#include <Epoch/Script/ScriptAsset.h>
#include <Epoch/Scene/SceneSerializer.h>
#include <Epoch/Assets/AssetImporter.h>
#include <Epoch/Assets/AssetExtensions.h>
#include <Epoch/Rendering/Material.h>
#include <Epoch/Editor/EditorSettings.h>
#include <Epoch/Editor/SelectionManager.h>
#include <Epoch/Editor/PanelIDs.h>
#include "Aeon/EditorResources.h"

namespace Epoch
{
	static bool staticOpenNewScriptPopup = false;
	static bool staticActivateSearchWidget = false;
	static bool staticOpenDeletePopup = false;

	void ContentBrowserPanel::Init()
	{
		staticInstance = this;
		memset(mySearchBuffer, 0, MAX_INPUT_BUFFER_LENGTH);

		myAssetIconMap[""]			= EditorResources::DirectoryIcon;
		myAssetIconMap[".fbx"]		= EditorResources::ModelIcon;
		myAssetIconMap[".gltf"]		= EditorResources::ModelIcon;
		myAssetIconMap[".glb"]		= EditorResources::ModelIcon;
		myAssetIconMap[".obj"]		= EditorResources::ModelIcon;
		myAssetIconMap[".png"]		= EditorResources::TextureIcon;
		myAssetIconMap[".jpg"]		= EditorResources::TextureIcon;
		myAssetIconMap[".jpeg"]		= EditorResources::TextureIcon;
		myAssetIconMap[".hdr"]		= EditorResources::TextureIcon;
		myAssetIconMap[".epoch"]	= EditorResources::SceneIcon;
		myAssetIconMap[".cs"]		= EditorResources::ScriptFileIcon;
		myAssetIconMap[".prefab"]	= EditorResources::PrefabIcon;
		myAssetIconMap[".mat"]		= EditorResources::MaterialIcon;
		myAssetIconMap[".mp4"]		= EditorResources::VideoIcon;
		myAssetIconMap[".ttf"]		= EditorResources::FontIcon;
	}

	void ContentBrowserPanel::OnImGuiRender(bool& aIsOpen)
	{
		EPOCH_PROFILE_FUNC();

		ImGui::Begin(CONTENT_BROWSER_PANEL_ID, 0, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
		{
			myIsContentBrowserHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
			myIsContentBrowserFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

			UI::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
			UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

			UI::ScopedStyle cellPadding(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 2.0f));

			ImGuiTableFlags tableFlags =
				ImGuiTableFlags_Resizable |
				ImGuiTableFlags_SizingFixedFit |
				ImGuiTableFlags_BordersInnerV;

			UI::PushID();
			if (ImGui::BeginTable("ContentBrowser_Table", 2, tableFlags, ImVec2(0.0f, 0.0f)))
			{
				ImGui::TableSetupColumn("Outliner", 0, 300.0f);
				ImGui::TableSetupColumn("Directory", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::BeginChild("##outliner");
				{
					UI::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
					UI::ScopedColorStack itemBg(ImGuiCol_Header, IM_COL32_DISABLE,
						ImGuiCol_HeaderActive, IM_COL32_DISABLE);

					if (myBaseDirectory)
					{
						// TODO: can we not sort this every frame?
						std::vector<std::shared_ptr<DirectoryInfo>> directories;
						directories.reserve(myBaseDirectory->subDirectories.size());
						for (auto& [handle, directory] : myBaseDirectory->subDirectories)
						{
							directories.emplace_back(directory);
						}

						std::sort(directories.begin(), directories.end(), [](const auto& a, const auto& b)
							{
								return a->filePath.stem().string() < b->filePath.stem().string();
							});

						RenderDirectoryHierarchy(myBaseDirectory, true);
					}
				}
				ImGui::EndChild();

				ImGui::TableSetColumnIndex(1);

				const float topBarHeight = 26.0f;
				const float bottomBarHeight = 32.0f;
				ImGui::BeginChild("##directory", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() - topBarHeight - bottomBarHeight));
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					RenderTopBar(topBarHeight);
					ImGui::PopStyleVar();

					//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
					ImGui::BeginChild("Scrolling");
					{

					}
					ImGui::EndChild();
					//ImGui::PopStyleColor();
				}
				ImGui::EndChild();

				if (ImGui::BeginDragDropTarget())
				{
					ImGui::EndDragDropTarget();
				}

				RenderBottomBar(bottomBarHeight);

				ImGui::EndTable();
			}
			UI::PopID();
		}
		ImGui::End();
	}

	void ContentBrowserPanel::OnEvent(Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& aEvent) { return OnKeyPressedEvent(aEvent); });
		dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& aEvent) { return OnMouseButtonPressed(aEvent); });
		dispatcher.Dispatch<EditorFileDroppedEvent>([this](EditorFileDroppedEvent& aEvent) { return OnFileDrop(aEvent); });
	}

	void ContentBrowserPanel::OnProjectChanged(const std::shared_ptr<Project>& aProject)
	{
		myDirectories.clear();
		myCurrentItems.Clear();
		myBaseDirectory = nullptr;
		myCurrentDirectory = nullptr;
		myNextDirectory = nullptr;
		myPreviousDirectory = nullptr;
		myBreadCrumbData.clear();

		SelectionManager::DeselectAll();

		AssetHandle baseDirectoryHandle = ProcessDirectory(Project::GetAssetDirectory().string(), nullptr);
		myBaseDirectory = myDirectories[baseDirectoryHandle];
		ChangeDirectory(myBaseDirectory);

		memset(mySearchBuffer, 0, MAX_INPUT_BUFFER_LENGTH);
	}

	std::shared_ptr<DirectoryInfo> ContentBrowserPanel::GetDirectory(const std::filesystem::path& aFilepath) const
	{
		if (aFilepath.string() == "" || aFilepath.string() == ".")
		{
			return myBaseDirectory;
		}

		for (const auto& [handle, directory] : myDirectories)
		{
			if (directory->filePath == aFilepath)
			{
				return directory;
			}
		}

		return nullptr;
	}

	AssetHandle ContentBrowserPanel::ProcessDirectory(const std::filesystem::path& aDirectoryPath, const std::shared_ptr<DirectoryInfo>& aParent)
	{
		const auto& directory = GetDirectory(aDirectoryPath);
		if (directory)
		{
			return directory->handle;
		}

		std::shared_ptr<DirectoryInfo> directoryInfo = std::make_shared<DirectoryInfo>();
		directoryInfo->handle = AssetHandle();
		directoryInfo->parent = aParent;

		if (aDirectoryPath == Project::GetAssetDirectory())
		{
			directoryInfo->filePath = Project::GetActive()->GetConfig().assetDirectory;
		}
		else
		{
			directoryInfo->filePath = std::filesystem::relative(aDirectoryPath, Project::GetAssetDirectory());
		}

		for (auto entry : std::filesystem::directory_iterator(aDirectoryPath))
		{
			if (entry.is_directory())
			{
				AssetHandle subdirHandle = ProcessDirectory(entry.path(), directoryInfo);
				directoryInfo->subDirectories[subdirHandle] = myDirectories[subdirHandle];
				continue;
			}

			auto metadata = Project::GetEditorAssetManager()->GetMetadata(std::filesystem::relative(entry.path(), Project::GetAssetDirectory()));
			if (!metadata.IsValid())
			{
				AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromPath(entry.path());
				if (type == AssetType::None)
				{
					continue;
				}

				metadata.handle = Project::GetEditorAssetManager()->ImportAsset(entry.path());
			}

			// Failed to import
			if (!metadata.IsValid())
			{
				continue;
			}

			directoryInfo->assets.push_back(metadata.handle);
		}

		myDirectories[directoryInfo->handle] = directoryInfo;
		return directoryInfo->handle;
	}

	void ContentBrowserPanel::ChangeDirectory(std::shared_ptr<DirectoryInfo>& aDirectory)
	{
		if (!aDirectory)
		{
			return;
		}

		myUpdateNavigationPath = true;

		myCurrentItems.items.clear();

		if (strlen(mySearchBuffer) == 0)
		{
			for (auto& [subdirHandle, subdir] : aDirectory->subDirectories)
			{
				myCurrentItems.items.push_back(std::make_shared<ContentBrowserDirectory>(subdir));
			}

			std::vector<AssetHandle> invalidAssets;
			for (auto assetHandle : aDirectory->assets)
			{
				AssetMetadata metadata = Project::GetEditorAssetManager()->GetMetadata(assetHandle);

				if (!metadata.IsValid())
				{
					continue;
				}

				auto itemIcon = EditorResources::OtherIcon;

				auto extension = metadata.filePath.extension().string();
				if (myAssetIconMap.find(extension) != myAssetIconMap.end())
				{
					itemIcon = myAssetIconMap[extension];
				}

				myCurrentItems.items.push_back(std::make_shared<ContentBrowserAsset>(metadata, itemIcon));
			}
		}
		else
		{
			myCurrentItems = Search(mySearchBuffer, aDirectory);
		}

		SortItemList();

		myPreviousDirectory = aDirectory;
		myCurrentDirectory = aDirectory;

		ClearSelections();
	}

	void ContentBrowserPanel::OnBrowseBack()
	{
		myNextDirectory = myCurrentDirectory;
		myPreviousDirectory = myCurrentDirectory->parent;
		ChangeDirectory(myPreviousDirectory);
	}

	void ContentBrowserPanel::OnBrowseForward()
	{
		ChangeDirectory(myNextDirectory);
	}

	void ContentBrowserPanel::RenderDirectoryHierarchy(std::shared_ptr<DirectoryInfo>& aDirectory, bool aDefaultOpen)
	{
		std::string name = aDirectory->filePath.filename().string();
		std::string id = name + "_TreeNode";
		bool previousState = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetID(id.c_str()));

		// ImGui item height hack
		auto* window = ImGui::GetCurrentWindow();
		window->DC.CurrLineSize.y = 20.0f;
		window->DC.CurrLineTextBaseOffset = 3.0f;


		const ImRect itemRect = { window->WorkRect.Min.x, window->DC.CursorPos.y, window->WorkRect.Max.x, window->DC.CursorPos.y + window->DC.CurrLineSize.y };

		const bool isItemClicked = [&itemRect, &id]
			{
				if (ImGui::ItemHoverable(itemRect, ImGui::GetID(id.c_str()), ImGuiItemFlags_None))
				{
					return ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left);
				}
				return false;
			}();

		const bool isWindowFocused = ImGui::IsWindowFocused();

		auto fillWithColour = [&](const ImColor& colour)
			{
				const ImU32 bgColour = ImGui::ColorConvertFloat4ToU32(colour);
				ImGui::GetWindowDrawList()->AddRectFilled(itemRect.Min, itemRect.Max, bgColour);
			};

		// Fill with light selection colour if any of the child entities selected
		//auto checkIfAnyDescendantSelected = [&](std::shared_ptr<DirectoryInfo>& aDirectory, auto aIsAnyDescendantSelected) -> bool
		//	{
		//		if (aDirectory->handle == myCurrentDirectory->handle)
		//			return true;
		//
		//		if (!aDirectory->subDirectories.empty())
		//		{
		//			for (auto& [childHandle, childDir] : aDirectory->subDirectories)
		//			{
		//				if (aIsAnyDescendantSelected(childDir, aIsAnyDescendantSelected))
		//				{
		//					return true;
		//				}
		//			}
		//		}
		//
		//		return false;
		//	};

		//const bool isAnyDescendantSelected = checkIfAnyDescendantSelected(aDirectory, checkIfAnyDescendantSelected);
		const bool isActiveDirectory = aDirectory->handle == myCurrentDirectory->handle;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		if (isActiveDirectory)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}
		if (aDirectory->subDirectories.size() == 0)
		{
			flags |= ImGuiTreeNodeFlags_Leaf;
		}
		if (aDefaultOpen)
		{
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		// Fill background
		//if (isActiveDirectory || isItemClicked)
		//{
		//	if (isWindowFocused)
		//	{
		//		fillWithColour(IM_COL32(237, 192, 119, 255));//TEMP
		//	}
		//	else
		//	{
		//		fillWithColour(IM_COL32(237, 192, 119, 255));//TEMP
		//	}
		//
		//	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(26, 26, 26, 255));
		//}
		//else if (isAnyDescendantSelected)
		//{
		//	fillWithColour(IM_COL32(237, 192, 119, 255));//TEMP
		//}

		// Tree Node
		//bool open = UI::TreeNode(id, name, flags, EditorResources::FolderIcon);
		bool open = ImGui::TreeNodeEx(id.c_str(), flags, name.c_str());
		bool clicked = ImGui::IsItemClicked();
		bool currentState = (flags &= ImGuiTreeNodeFlags_Leaf) ? false : open;

		//if (isActiveDirectory || isItemClicked)
		//{
		//	ImGui::PopStyleColor();
		//}

		// Fixing slight overlap
		UI::ShiftCursorY(3.0f);

		// Draw children
		if (open)
		{
			std::vector<std::shared_ptr<DirectoryInfo>> directories;
			directories.reserve(myBaseDirectory->subDirectories.size());
			for (auto& [handle, directory] : aDirectory->subDirectories)
			{
				directories.emplace_back(directory);
			}

			std::sort(directories.begin(), directories.end(), [](const auto& a, const auto& b)
				{
					return a->filePath.stem().string() < b->filePath.stem().string();
				});

			for (auto& child : directories)
			{
				RenderDirectoryHierarchy(child);
			}
		}

		if (!isActiveDirectory && currentState == previousState && clicked)
		{
			ChangeDirectory(aDirectory);
		}

		if (open)
		{
			ImGui::TreePop();
		}
	}

	void ContentBrowserPanel::RenderTopBar(float aHeight)
	{
		ImGui::BeginChild("##top_bar", ImVec2(0, aHeight));
		//ImGui::BeginHorizontal("##top_bar", ImGui::GetWindowSize());
		{
			// Navigation buttons
			{
				UI::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

				ImGui::SameLine();
				if (ImGui::Button("<", { aHeight, aHeight }))
				{
					OnBrowseBack();
				}
				UI::SetTooltip("Previous directory");

				ImGui::SameLine();
				if (ImGui::Button(">", { aHeight, aHeight }))
				{
					OnBrowseForward();
				}
				UI::SetTooltip("Next directory");
			}

			// Search
			{
				const float searchBarWidth = 400;
				ImGui::SameLine();

				//UI::ShiftCursorY(2.0f);
				ImGui::SetNextItemWidth(searchBarWidth);

				if (staticActivateSearchWidget)
				{
					ImGui::SetKeyboardFocusHere();
					staticActivateSearchWidget = false;
				}

				if (ImGui::InputTextWithHint("##AssetSearch", "Search...", mySearchBuffer, MAX_INPUT_BUFFER_LENGTH))
				{
					if (strlen(mySearchBuffer) == 0)
					{
						ChangeDirectory(myCurrentDirectory);
					}
					else
					{
						myCurrentItems = Search(mySearchBuffer, myCurrentDirectory);
						SortItemList();
					}
				}
				//UI::ShiftCursorY(-2.0f);
			}

			if (myUpdateNavigationPath)
			{
				myBreadCrumbData.clear();

				std::shared_ptr<DirectoryInfo> current = myCurrentDirectory;
				while (current && current->parent != nullptr)
				{
					myBreadCrumbData.push_back(current);
					current = current->parent;
				}

				std::reverse(myBreadCrumbData.begin(), myBreadCrumbData.end());
				myUpdateNavigationPath = false;
			}

			// Breadcrumbs
			{
				ImGui::SameLine();

				UI::ScopedFont boldFont("Bold");
				UI::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 0.0f));

				const std::string& assetsDirectoryName = Project::GetActive()->GetConfig().assetDirectory.string();
				ImVec2 textSize = ImGui::CalcTextSize(assetsDirectoryName.c_str());
				const float textPadding = ImGui::GetStyle().FramePadding.y;
				if (ImGui::Selectable(assetsDirectoryName.c_str(), false, 0, ImVec2(textSize.x, textSize.y + textPadding)))
				{
					SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
					ChangeDirectory(myBaseDirectory);
				}

				for (auto& directory : myBreadCrumbData)
				{
					ImGui::SameLine();
					ImGui::Text("/");
					ImGui::SameLine();

					std::string directoryName = directory->filePath.filename().string();
					ImVec2 textSize = ImGui::CalcTextSize(directoryName.c_str());
					if (ImGui::Selectable(directoryName.c_str(), false, 0, ImVec2(textSize.x, textSize.y + textPadding)))
					{
						SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
						ChangeDirectory(directory);
					}
				}
			}
		}
		//ImGui::EndHorizontal();
		ImGui::EndChild();
	}

	void ContentBrowserPanel::RenderItems()
	{
	}

	void ContentBrowserPanel::RenderBottomBar(float aHeight)
	{
		UI::ScopedStyle childBorderSize(ImGuiStyleVar_ChildBorderSize, 0.0f);
		UI::ScopedStyle frameBorderSize(ImGuiStyleVar_FrameBorderSize, 0.0f);
		UI::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		UI::ScopedStyle framePadding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		
		ImGui::BeginChild("##bottom_bar", ImVec2(0, aHeight));
		//ImGui::BeginHorizontal("##bottom_bar");
		{
			size_t selectionCount = SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser);
			if (selectionCount == 1)
			{
				AssetHandle firstSelection = SelectionManager::GetSelections(SelectionContext::ContentBrowser)[0];
				const auto& assetMetadata = Project::GetEditorAssetManager()->GetMetadata(firstSelection);

				std::string assetDirectory = Project::GetActive()->GetConfig().assetDirectory.string();

				std::string filepath = "";
				if (assetMetadata.IsValid())
				{
					filepath = assetDirectory + "/" + assetMetadata.filePath.string();
				}
				else if (myDirectories.find(firstSelection) != myDirectories.end())
				{
					filepath = assetDirectory + "/" + myDirectories[firstSelection]->filePath.string();
				}

				std::replace(filepath.begin(), filepath.end(), '\\', '/');
				ImGui::TextUnformatted(filepath.c_str());
			}
			else if (selectionCount > 1)
			{
				ImGui::Text("%d items selected", selectionCount);
			}
		}
		//ImGui::EndHorizontal();
		ImGui::EndChild();
	}

	void ContentBrowserPanel::UpdateInput()
	{
		if (!myIsContentBrowserHovered)
		{
			return;
		}

		if ((!myIsAnyItemHovered && (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || Input::IsKeyPressed(KeyCode::Escape)))
		{
			ClearSelections();
		}
	}

	bool ContentBrowserPanel::OnKeyPressedEvent(KeyPressedEvent& aEvent)
	{
		if (!myIsContentBrowserFocused)
		{
			return false;
		}

		bool handled = false;

		if (Input::IsKeyHeld(KeyCode::LeftControl))
		{
			switch (aEvent.GetKeyCode())
			{
			case KeyCode::C:
			{
				myCopiedAssets.CopyFrom(SelectionManager::GetSelections(SelectionContext::ContentBrowser));
				handled = true;
				break;
			}
			case KeyCode::V:
			{
				PasteCopiedAssets();
				handled = true;
				break;
			}
			case KeyCode::D:
			{
				myCopiedAssets.CopyFrom(SelectionManager::GetSelections(SelectionContext::ContentBrowser));
				PasteCopiedAssets();
				handled = true;
				break;
			}
			case KeyCode::F:
			{
				staticActivateSearchWidget = true;
				break;
			}
			}

			if (Input::IsKeyHeld(KeyCode::LeftShift))
			{
				switch (aEvent.GetKeyCode())
				{
				case KeyCode::N:
				{
					std::filesystem::path filepath = FileSystem::GetUniqueFileName(Project::GetAssetDirectory() / myCurrentDirectory->filePath / "New Folder");

					// NOTE: For some reason creating new directories through code doesn't trigger a file system change?
					bool created = FileSystem::CreateDirectory(filepath);

					if (created)
					{
						Refresh();
						const auto& directoryInfo = GetDirectory(myCurrentDirectory->filePath / filepath.filename());
						size_t index = myCurrentItems.FindItem(directoryInfo->handle);
						if (index != ContentBrowserItemList::InvalidItem)
						{
							SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
							SelectionManager::Select(SelectionContext::ContentBrowser, directoryInfo->handle);
							myCurrentItems[index]->StartRenaming();
						}
					}
					handled = true;
				}
				break;
				}
			}
		}

		if (Input::IsKeyHeld(KeyCode::LeftAlt) || Input::IsKeyHeld(KeyCode::RightAlt))
		{
			switch (aEvent.GetKeyCode())
			{
			case KeyCode::Left:
			{
				OnBrowseBack();
				handled = true;
				break;
			}
			case KeyCode::Right:
			{
				OnBrowseForward();
				handled = true;
				break;
			}
			}
		}

		if (aEvent.GetKeyCode() == KeyCode::Delete && SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) > 0)
		{
			for (const auto& item : myCurrentItems)
			{
				if (item->IsRenaming())
				{
					return false;
				}
			}

			staticOpenDeletePopup = true;
			handled = true;
		}

		if (aEvent.GetKeyCode() == KeyCode::F5)
		{
			Refresh();
			handled = true;
		}

		return handled;
	}

	bool ContentBrowserPanel::OnMouseButtonPressed(MouseButtonPressedEvent& aEvent)
	{
		if (!myIsContentBrowserFocused) return false;

		bool handled = false;

		switch (aEvent.GetMouseButton())
		{
		case MouseButton::Button3:
			OnBrowseBack();
			handled = true;
			break;
		case MouseButton::Button4:
			OnBrowseForward();
			handled = true;
			break;
		}

		return handled;
	}

	bool ContentBrowserPanel::OnFileDrop(EditorFileDroppedEvent& aEvent)
	{
		if (aEvent.GetPaths().size() > 0)
		{
			std::vector<std::filesystem::path> paths(aEvent.GetPaths().size());
			for (const std::string& path : aEvent.GetPaths())
			{
				paths.push_back(path);
			}

			Import(paths);
		}

		return true;
	}

	void ContentBrowserPanel::Import(const std::vector<std::filesystem::path>& aPaths)
	{
		bool imported = false;
		for (const std::filesystem::path& path : aPaths)
		{
			imported |= FileSystem::CopyFile(path, Project::GetAssetDirectory() / myCurrentDirectory->filePath);
		}

		if (imported)
		{
			Refresh();
		}
	}

	void ContentBrowserPanel::Refresh()
	{
		myCurrentItems.Clear();
		myDirectories.clear();

		std::shared_ptr<DirectoryInfo> currentDirectory = myCurrentDirectory;
		AssetHandle baseDirectoryHandle = ProcessDirectory(Project::GetAssetDirectory().string(), nullptr);
		myBaseDirectory = myDirectories[baseDirectoryHandle];
		myCurrentDirectory = GetDirectory(currentDirectory->filePath);

		if (!myCurrentDirectory)
		{
			myCurrentDirectory = myBaseDirectory; // Our current directory was removed
		}

		ChangeDirectory(myCurrentDirectory);
	}

	void ContentBrowserPanel::ClearSelections()
	{
		std::vector<AssetHandle> selectedItems = SelectionManager::GetSelections(SelectionContext::ContentBrowser);
		for (AssetHandle itemHandle : selectedItems)
		{
			size_t index = myCurrentItems.FindItem(itemHandle);

			if (index == ContentBrowserItemList::InvalidItem)
			{
				continue;
			}

			SelectionManager::Deselect(SelectionContext::ContentBrowser, itemHandle);

			if (myCurrentItems[index]->IsRenaming())
			{
				myCurrentItems[index]->StopRenaming();
			}
		}
	}

	void ContentBrowserPanel::PasteCopiedAssets()
	{
		if (myCopiedAssets.SelectionCount() == 0) return;

		for (AssetHandle copiedAsset : myCopiedAssets)
		{
			size_t assetIndex = myCurrentItems.FindItem(copiedAsset);

			if (assetIndex == ContentBrowserItemList::InvalidItem)
			{
				continue;
			}

			const auto& item = myCurrentItems[assetIndex];
			auto originalFilePath = Project::GetAssetDirectory();

			if (item->GetType() == ContentBrowserItem::ItemType::Asset)
			{
				originalFilePath /= std::static_pointer_cast<ContentBrowserAsset>(item)->GetAssetInfo().filePath;
				auto filepath = FileSystem::GetUniqueFileName(originalFilePath);
				EPOCH_ASSERT(!std::filesystem::exists(filepath));
				std::filesystem::copy_file(originalFilePath, filepath);
			}
			else
			{
				originalFilePath /= std::static_pointer_cast<ContentBrowserDirectory>(item)->GetDirectoryInfo()->filePath;
				auto filepath = FileSystem::GetUniqueFileName(originalFilePath);
				EPOCH_ASSERT(!std::filesystem::exists(filepath));
				std::filesystem::copy(originalFilePath, filepath, std::filesystem::copy_options::recursive);
			}
		}

		Refresh();

		SelectionManager::DeselectAll();
		myCopiedAssets.Clear();
	}

	void ContentBrowserPanel::RenderDeleteModal()
	{
		//UI::Fonts::PushFont("Bold");
		//if (ImGui::BeginPopupModal(" Delete selected asset(s)?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		//{
		//	UI::Fonts::PushFont("Regular");
		//
		//	float width = ImGui::GetContentRegionAvail().x;
		//	ImGui::BeginChild("Files to delete", ImVec2(width, 70.0f));
		//	for (const DirectoryEntry& entry : myDirectoryEntries)
		//	{
		//		if (!entry.isSelected)
		//		{
		//			continue;
		//		}
		//
		//		std::string filePath = "  " + std::filesystem::relative(entry.entry.path(), myBaseDirectory.parent_path()).string();
		//		ImGui::Text(filePath.c_str());
		//	}
		//	ImGui::EndChild();
		//
		//	UI::Spacing();
		//	ImGui::Separator();
		//	UI::Fonts::PushFont("Bold");
		//	ImGui::Text(" You cannot undo this action.");
		//	UI::Fonts::PopFont();
		//	UI::Spacing();
		//	ImGui::Separator();
		//
		//	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6196f, 0.1373f, 0.1373f, 1));
		//	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7196f, 0.2373f, 0.2373f, 1));
		//	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5196f, 0.0373f, 0.0373f, 1));
		//	if (ImGui::Button("Delete", ImVec2(120, 0)))
		//	{
		//		DeleteSelected();
		//		ImGui::CloseCurrentPopup();
		//	}
		//	ImGui::PopStyleColor(3);
		//
		//	ImGui::SetItemDefaultFocus();
		//	ImGui::SameLine();
		//
		//	if (ImGui::Button("Cancel", ImVec2(120, 0)))
		//	{
		//		ImGui::CloseCurrentPopup();
		//	}
		//
		//	UI::Fonts::PopFont();
		//	ImGui::EndPopup();
		//}
		//UI::Fonts::PopFont();
	}

	void ContentBrowserPanel::RenderNewScriptDialogue()
	{
		UI::Fonts::PushFont("Bold");
		if (ImGui::BeginPopupModal("New Script", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			UI::Fonts::PushFont("Regular");

			static char* newScriptNamespaceBuffer = new char[256];
			static char* newScriptNameBuffer = new char[256];
			static bool inited = false;
			if (!inited)
			{
				inited = true;
				memset(newScriptNamespaceBuffer, 0, 256);
				memset(newScriptNameBuffer, 0, 256);
			}
			
			const std::string namespaceString = std::string(newScriptNamespaceBuffer).empty() ? Project::GetActive()->GetConfig().defaultScriptNamespace : std::string(newScriptNamespaceBuffer);

			UI::Fonts::PushFont("Bold");
			const std::string fullProjectPath =  namespaceString + "." + newScriptNameBuffer;
			ImGui::Text(fullProjectPath.c_str());
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			UI::Fonts::PopFont();

			ImGui::InputTextWithHint("##new_script_namespace", "Namespace...", newScriptNamespaceBuffer, 256);
			ImGui::InputTextWithHint("##new_script_name", "Script Name...", newScriptNameBuffer, 256);

			const bool fileAlreadyExists = FileSystem::Exists(Project::GetAssetDirectory() / myCurrentDirectory->filePath / (std::string(newScriptNameBuffer) + ".cs"));
			ImGui::BeginDisabled(strlen(newScriptNameBuffer) == 0 || fileAlreadyExists);
			if (ImGui::Button("Create", ImVec2(75, 0)))
			{
				CreateAsset<ScriptFileAsset>(std::string(newScriptNameBuffer) + ".cs", namespaceString.c_str(), newScriptNameBuffer);

				memset(newScriptNamespaceBuffer, 0, 255);
				memset(newScriptNameBuffer, 0, 255);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndDisabled();

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(75, 0)))
			{
				memset(newScriptNameBuffer, 0, 255);
				ImGui::CloseCurrentPopup();
			}

			UI::Fonts::PopFont();
			ImGui::EndPopup();
		}
		UI::Fonts::PopFont();
	}
	
	void ContentBrowserPanel::SortItemList()
	{
		std::sort(myCurrentItems.begin(), myCurrentItems.end(), [](const std::shared_ptr<ContentBrowserItem>& aItem0, const std::shared_ptr<ContentBrowserItem>& aItem1)
			{
				if (aItem0->GetType() == aItem1->GetType())
				{
					return CU::ToLower(aItem0->GetName()) < CU::ToLower(aItem1->GetName());
				}

				return (uint16_t)aItem0->GetType() < (uint16_t)aItem1->GetType();
			});
	}
	
	ContentBrowserItemList ContentBrowserPanel::Search(const std::string& aQuery, const std::shared_ptr<DirectoryInfo>& aDirectoryInfo)
	{
		ContentBrowserItemList results;
		std::string queryLowerCase = CU::ToLower(aQuery);

		for (auto& [handle, subdir] : aDirectoryInfo->subDirectories)
		{
			std::string subdirName = subdir->filePath.filename().string();
			if (subdirName.find(queryLowerCase) != std::string::npos)
			{
				results.items.push_back(std::make_shared<ContentBrowserDirectory>(subdir));
			}

			ContentBrowserItemList list = Search(aQuery, subdir);
			results.items.insert(results.items.end(), list.items.begin(), list.items.end());
		}

		for (auto& assetHandle : aDirectoryInfo->assets)
		{
			auto& asset = Project::GetEditorAssetManager()->GetMetadata(assetHandle);
			std::string filename = CU::ToLower(asset.filePath.filename().string());

			if (filename.find(queryLowerCase) != std::string::npos)
			{
				results.items.push_back(std::make_shared<ContentBrowserAsset>(asset, myAssetIconMap.find(asset.filePath.extension().string()) != myAssetIconMap.end() ? myAssetIconMap[asset.filePath.extension().string()] : EditorResources::OtherIcon));
			}
		}

		return results;
	}
}
