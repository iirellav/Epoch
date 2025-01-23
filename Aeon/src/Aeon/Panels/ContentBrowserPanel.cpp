#include "ContentBrowserPanel.h"
#include <fstream>
#include <CommonUtilities/Math/CommonMath.hpp>
#include <CommonUtilities/StringUtils.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Core/Input.h>
#include <Epoch/Debug/Log.h>
#include <Epoch/Debug/Profiler.h>
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
	static float staticPadding = 2.0f;

	static char staticDeleteModalName[] = " Delete selected asset(s)?";
	static char staticNewScriptModalName[] = " New Script";

	ContentBrowserPanel::ContentBrowserPanel()
	{
		staticInstance = this;
		memset(mySearchBuffer, 0, MAX_INPUT_BUFFER_LENGTH);

		myAssetIconMap[""] = EditorResources::ClosedFolderIcon;
		myAssetIconMap[".fbx"] = EditorResources::ModelIcon;
		myAssetIconMap[".gltf"] = EditorResources::ModelIcon;
		myAssetIconMap[".glb"] = EditorResources::ModelIcon;
		myAssetIconMap[".obj"] = EditorResources::ModelIcon;
		myAssetIconMap[".png"] = EditorResources::TextureIcon;
		myAssetIconMap[".jpg"] = EditorResources::TextureIcon;
		myAssetIconMap[".jpeg"] = EditorResources::TextureIcon;
		myAssetIconMap[".hdr"] = EditorResources::TextureIcon;
		myAssetIconMap[".epoch"] = EditorResources::SceneIcon;
		myAssetIconMap[".cs"] = EditorResources::ScriptFileIcon;
		myAssetIconMap[".prefab"] = EditorResources::PrefabIcon;
		myAssetIconMap[".mat"] = EditorResources::MaterialIcon;
		myAssetIconMap[".mp4"] = EditorResources::VideoIcon;
		myAssetIconMap[".ttf"] = EditorResources::FontIcon;
	}

	void ContentBrowserPanel::OnImGuiRender(bool& aIsOpen)
	{
		EPOCH_PROFILE_FUNC();

		ImGui::Begin(CONTENT_BROWSER_PANEL_ID, 0, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
		{
			if (myShouldRefresh)
			{
				Refresh();
				myShouldRefresh = false;
			}

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
				ImGui::TableSetupColumn("Outliner", 0, 200.0f);
				ImGui::TableSetupColumn("Directory", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::BeginChild("##outliner");
				{
					UI::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
					//UI::ScopedColorStack itemBg(ImGuiCol_Header, IM_COL32_DISABLE, ImGuiCol_HeaderActive, IM_COL32_DISABLE);

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

						RenderDirectoryHierarchy(myBaseDirectory/*, true*/);
					}
				}
				ImGui::EndChild();

				ImGui::TableSetColumnIndex(1);

				const float topBarHeight = 26.0f;
				const float bottomBarHeight = 38.0f;
				ImGui::BeginChild("##directory", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() - topBarHeight - bottomBarHeight));
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					RenderTopBar(topBarHeight);
					ImGui::PopStyleVar();
					
					ImGui::Separator();

					ImGui::BeginChild("Scrolling");
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.35f));

						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
						if (ImGui::BeginPopupContextWindow("CBContextMenu", ImGuiPopupFlags_MouseButtonRight))
						{
							if (ImGui::BeginMenu("Create"))
							{
								if (ImGui::MenuItem("Folder"))
								{
									std::filesystem::path filepath = FileSystem::GetUniqueFileName(Project::GetAssetDirectory() / myCurrentDirectory->filePath / "New Folder");

									bool created = FileSystem::CreateDirectory(filepath);
									if (created)
									{
										Refresh();
										auto folderPath = filepath.filename();
										if (myCurrentDirectory->filePath != myBaseDirectory->filePath)
										{
											folderPath = myCurrentDirectory->filePath / folderPath;
										}
										const auto& directoryInfo = GetDirectory(folderPath);
										size_t index = myCurrentItems.FindItem(directoryInfo->handle);
										if (index != ContentBrowserItemList::InvalidItem)
										{
											SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
											SelectionManager::Select(SelectionContext::ContentBrowser, directoryInfo->handle);
											myCurrentItems[index]->StartRenaming();
										}
									}
								}

								if (ImGui::MenuItem("Scene"))
								{
									auto asset = CreateAsset<Scene>("New Scene.epoch");
									if (asset)
									{
										size_t index = myCurrentItems.FindItem(asset->GetHandle());
										if (index != ContentBrowserItemList::InvalidItem)
										{
											SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
											SelectionManager::Select(SelectionContext::ContentBrowser, asset->GetHandle());
											myCurrentItems[index]->StartRenaming();
										}
									}
								}

								if (ImGui::MenuItem("Material"))
								{
									auto asset = CreateAsset<Material>("New Material.mat");
									if (asset)
									{
										size_t index = myCurrentItems.FindItem(asset->GetHandle());
										if (index != ContentBrowserItemList::InvalidItem)
										{
											SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
											SelectionManager::Select(SelectionContext::ContentBrowser, asset->GetHandle());
											myCurrentItems[index]->StartRenaming();
										}
									}
								}

								if (ImGui::MenuItem("Script"))
								{
									staticOpenNewScriptPopup = true;
								}

								ImGui::EndMenu();
							}

							ImGui::Separator();

							if (ImGui::MenuItem("Import"))
							{
								std::vector<std::filesystem::path> assetPaths = FileSystem::OpenFileDialogMultiple({ { "Asset", "png,jpg,jpeg,hdr,fbx,obj,gltf,glb,ttf" } }, myCurrentDirectory->filePath.string().c_str());
								Import(assetPaths);
							}

							ImGui::Separator();

							if (ImGui::MenuItem("Copy", "Ctrl+C", nullptr, SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) > 0))
							{
								myCopiedAssets.CopyFrom(SelectionManager::GetSelections(SelectionContext::ContentBrowser));
							}

							if (ImGui::MenuItem("Paste", "Ctrl+V", nullptr, myCopiedAssets.SelectionCount() > 0))
							{
								PasteCopiedAssets();
							}

							if (ImGui::MenuItem("Duplicate", "Ctrl+D", nullptr, SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) > 0))
							{
								myCopiedAssets.CopyFrom(SelectionManager::GetSelections(SelectionContext::ContentBrowser));
								PasteCopiedAssets();
							}

							ImGui::Separator();
							
							if (ImGui::MenuItem("Show in Explorer"))
							{
								FileSystem::OpenDirectoryInExplorer(Project::GetAssetDirectory() / myCurrentDirectory->filePath);
							}

							ImGui::EndPopup();
						}
						ImGui::PopStyleVar();

						{
							const float scrollBarrOffset = 20.0f + ImGui::GetStyle().ScrollbarSize;
							float panelWidth = ImGui::GetContentRegionAvail().x - scrollBarrOffset;
							float cellSize = EditorSettings::Get().contentBrowserThumbnailSize + staticPadding;
							int columnCount = (int)(panelWidth / cellSize);
							if (columnCount < 1) columnCount = 1;

							const float rowSpacing = 12.0f;
							UI::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(staticPadding, rowSpacing));
							ImGui::Columns(columnCount, 0, false);

							UI::ScopedStyle border(ImGuiStyleVar_FrameBorderSize, 0.0f);
							UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
							RenderItems();
						}

						if (ImGui::IsWindowFocused() && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
						{
							UpdateInput();
						}

						ImGui::PopStyleColor(2);

						RenderDeleteDialogue();
						RenderNewScriptDialogue();
					}
					ImGui::EndChild();
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
			directoryInfo->filePath = "";
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

				Project::GetEditorAssetManager()->ImportAsset(entry.path());
				metadata = Project::GetEditorAssetManager()->GetMetadata(std::filesystem::relative(entry.path(), Project::GetAssetDirectory()));
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

		ClearSelections();
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
		if (name == "")
		{
			name = Project::GetAssetDirectory().stem().string();
		}
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

		// Tree Node
		auto folderIcon = (aDirectory->subDirectories.size() == 0 || previousState == false) ? EditorResources::ClosedFolderIcon : EditorResources::OpenFolderIcon;
		bool open = UI::TreeNode(id, name, flags, folderIcon);
		bool clicked = ImGui::IsItemClicked();
		bool currentState = (flags &= ImGuiTreeNodeFlags_Leaf) ? false : open;
		bool active = ImGui::IsItemActive();

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

		UpdateDropArea(aDirectory);

		if ((!isActiveDirectory && currentState == previousState && !ImGui::IsMouseDragging(ImGuiMouseButton_Left) && clicked) ||
			(!isActiveDirectory && active && Input::IsKeyPressed(KeyCode::Enter)))
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

				auto contenBrowserButton = [aHeight](const char* labelId, const std::shared_ptr<Texture2D>& icon)
					{
						UI::ScopedColorStack buttonColors(
							ImGuiCol_Button, IM_COL32(255, 255, 255, 0),
							ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 0),
							ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 0));

						const float iconSize = CU::Math::Min(24.0f, aHeight);
						const float iconPadding = 3.0f;
						const bool clicked = ImGui::Button(labelId, ImVec2(iconSize, iconSize));
						UI::Draw::DrawButtonImage(icon,
							IM_COL32(255, 255, 255, 200), IM_COL32(255, 255, 255, 255), IM_COL32(255, 255, 255, 150),
							UI::RectExpanded(UI::GetItemRect(), -iconPadding, -iconPadding));

						return clicked;
					};

				ImGui::SameLine();
				if (contenBrowserButton("##back", EditorResources::BackIcon))
				{
					OnBrowseBack();
				}
				UI::SetTooltip("Previous directory");

				ImGui::SameLine();
				if (contenBrowserButton("##forward", EditorResources::ForwardIcon))
				{
					OnBrowseForward();
				}
				UI::SetTooltip("Next directory");

				ImGui::SameLine();
				if (contenBrowserButton("##refresh", EditorResources::RefreshIcon))
				{
					Refresh();
				}
				UI::SetTooltip("Refresh");
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
				UpdateDropArea(myBaseDirectory);

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
					UpdateDropArea(directory);
				}
			}

			// Size slider
			{
				ImGui::SameLine();

				const float sliderWidth = 200.0f;
				UI::ShiftCursorX(ImGui::GetContentRegionAvail().x - sliderWidth);

				ImGui::SetNextItemWidth(sliderWidth);
				ImGui::SliderFloat("##CellSize", &EditorSettings::Get().contentBrowserThumbnailSize, 64.0f, 256.0f, "", ImGuiSliderFlags_AlwaysClamp);
			}
		}
		//ImGui::EndHorizontal();
		ImGui::EndChild();
	}

	void ContentBrowserPanel::RenderItems()
	{
		myIsAnyItemHovered = false;

		for (auto& item : myCurrentItems)
		{
			item->OnRenderBegin();
			CBItemActionResult result = item->OnRender();
			item->OnRenderEnd();

			if (result.IsSet(ContentBrowserAction::ClearSelections))
			{
				ClearSelections();
			}

			if (result.IsSet(ContentBrowserAction::Deselected))
			{
				SelectionManager::Deselect(SelectionContext::ContentBrowser, item->GetID());
			}

			if (result.IsSet(ContentBrowserAction::Selected))
			{
				SelectionManager::Select(SelectionContext::ContentBrowser, item->GetID());
			}

			if (result.IsSet(ContentBrowserAction::SelectToHere) && SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) == 2)
			{
				size_t firstIndex = myCurrentItems.FindItem(SelectionManager::GetSelections(SelectionContext::ContentBrowser)[0]);
				size_t lastIndex = myCurrentItems.FindItem(item->GetID());

				if (firstIndex > lastIndex)
				{
					size_t temp = firstIndex;
					firstIndex = lastIndex;
					lastIndex = temp;
				}

				for (size_t i = firstIndex; i <= lastIndex; i++)
				{
					SelectionManager::Select(SelectionContext::ContentBrowser, myCurrentItems[i]->GetID());
				}
			}

			if (result.IsSet(ContentBrowserAction::StartRenaming))
			{
				item->StartRenaming();
			}

			if (result.IsSet(ContentBrowserAction::Copy))
			{
				myCopiedAssets.CopyFrom(SelectionManager::GetSelections(SelectionContext::ContentBrowser));
				//myCopiedAssets.Select(item->GetID());
			}

			if (result.IsSet(ContentBrowserAction::Reload))
			{
				AssetManager::ReloadData(item->GetID());
			}

			if (result.IsSet(ContentBrowserAction::OpenDeleteDialogue) && !item->IsRenaming())
			{
				staticOpenDeletePopup = true;
			}

			if (result.IsSet(ContentBrowserAction::ShowInExplorer))
			{
				if (item->GetType() == ContentBrowserItem::ItemType::Directory)
				{
					FileSystem::ShowFileInExplorer(Project::GetAssetDirectory() / myCurrentDirectory->filePath / item->GetName());
				}
				else
				{
					FileSystem::ShowFileInExplorer(Project::GetEditorAssetManager()->GetFileSystemPath(Project::GetEditorAssetManager()->GetMetadata(item->GetID())));
				}
			}

			if (result.IsSet(ContentBrowserAction::OpenExternal))
			{
				if (item->GetType() == ContentBrowserItem::ItemType::Directory)
				{
					FileSystem::OpenExternally(Project::GetAssetDirectory() / myCurrentDirectory->filePath / item->GetName());
				}
				else
				{
					FileSystem::OpenExternally(Project::GetEditorAssetManager()->GetFileSystemPath(Project::GetEditorAssetManager()->GetMetadata(item->GetID())));
				}
			}

			if (result.IsSet(ContentBrowserAction::Hovered))
			{
				myIsAnyItemHovered = true;
			}

			if (result.IsSet(ContentBrowserAction::Duplicate))
			{
				myCopiedAssets.CopyFrom(SelectionManager::GetSelections(SelectionContext::ContentBrowser));
				//myCopiedAssets.Select(item->GetID());
				PasteCopiedAssets();
				break;
			}

			if (result.IsSet(ContentBrowserAction::Renamed))
			{
				SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
				Refresh();
				SortItemList();

				if (item->GetID() == mySceneContext->GetHandle() && myCurrentSceneRenamedCallback)
				{
					auto assetItem = std::static_pointer_cast<ContentBrowserAsset>(item);
					const auto& assetMetadata = assetItem->GetAssetInfo();
					myCurrentSceneRenamedCallback(assetMetadata);
				}

				break;
			}

			if (result.IsSet(ContentBrowserAction::Activated))
			{
				if (item->GetType() == ContentBrowserItem::ItemType::Directory)
				{
					SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
					ChangeDirectory(std::static_pointer_cast<ContentBrowserDirectory>(item)->GetDirectoryInfo());
					break;
				}
				else
				{
					auto assetItem = std::static_pointer_cast<ContentBrowserAsset>(item);
					const auto& assetMetadata = assetItem->GetAssetInfo();

					if (myItemActivationCallbacks.find(assetMetadata.type) != myItemActivationCallbacks.end())
					{
						myItemActivationCallbacks[assetMetadata.type](assetMetadata);
					}
				}
			}

			if (result.IsSet(ContentBrowserAction::Refresh))
			{
				Refresh();
				break;
			}
		}

		if (staticOpenDeletePopup)
		{
			ImGui::OpenPopup(staticDeleteModalName);
			staticOpenDeletePopup = false;
		}

		if (staticOpenNewScriptPopup)
		{
			ImGui::OpenPopup(staticNewScriptModalName);
			staticOpenNewScriptPopup = false;
		}
	}

	void ContentBrowserPanel::RenderBottomBar(float aHeight)
	{
		UI::ScopedStyle childBorderSize(ImGuiStyleVar_ChildBorderSize, 0.0f);
		UI::ScopedStyle frameBorderSize(ImGuiStyleVar_FrameBorderSize, 0.0f);
		UI::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		UI::ScopedStyle framePadding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		
		ImGui::BeginChild("##bottom_bar", ImVec2(ImGui::GetWindowSize().x, aHeight));
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

		if ((!myIsAnyItemHovered && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) || Input::IsKeyPressed(KeyCode::Escape)))
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
			if (path.extension() == ".gltf")
			{
				auto binPath = CU::RemoveExtension(path.string()) + ".bin";
				FileSystem::CopyFile(binPath, Project::GetAssetDirectory() / myCurrentDirectory->filePath);
			}
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

	void ContentBrowserPanel::RenderDeleteDialogue()
	{
		UI::Fonts::PushFont("Bold");
		if (ImGui::BeginPopupModal(staticDeleteModalName, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			UI::Fonts::PushFont("Regular");
		
			if (SelectionManager::GetSelectionCount(SelectionContext::ContentBrowser) == 0)
			{
				ImGui::CloseCurrentPopup();
			}

			float width = ImGui::GetContentRegionAvail().x;
			auto selectedItems = SelectionManager::GetSelections(SelectionContext::ContentBrowser);
			ImGui::BeginChild("Files to delete", ImVec2(width, 100.0f));
			for (AssetHandle handle : selectedItems)
			{
				size_t index = myCurrentItems.FindItem(handle);
				if (index == ContentBrowserItemList::InvalidItem)
				{
					continue;
				}
				auto& item = myCurrentItems[index];
				UI::Draw::Image(item->GetIcon(), ImVec2(18, 18));
				ImGui::SameLine();
				ImGui::Text(item->GetName().c_str());
			}
			ImGui::EndChild();
		
			ImGui::Separator();
			UI::Fonts::PushFont("Bold");
			ImGui::Text(" You cannot undo this action.");
			UI::Fonts::PopFont();
			ImGui::Separator();
		
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6196f, 0.1373f, 0.1373f, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7196f, 0.2373f, 0.2373f, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5196f, 0.0373f, 0.0373f, 1));
			if (ImGui::Button("Delete", ImVec2(120.0f, 0.0f)))
			{
				std::vector<AssetMetadata> deletedAssetMetadata;

				auto selectedItems = SelectionManager::GetSelections(SelectionContext::ContentBrowser);
				for (AssetHandle handle : selectedItems)
				{
					size_t index = myCurrentItems.FindItem(handle);
					if (index == ContentBrowserItemList::InvalidItem)
					{
						continue;
					}

					myCurrentItems[index]->Delete();
					myCurrentItems.Erase(handle);

					const auto& metaData = Project::GetEditorAssetManager()->GetMetadata(handle);
					if (metaData.IsValid())
					{
						deletedAssetMetadata.push_back(metaData);
					}
				}

				for (const auto& deletedMetadata : deletedAssetMetadata)
				{
					if (myNewAssetCreatedCallbacks.find(deletedMetadata.type) != myNewAssetCreatedCallbacks.end())
					{
						myNewAssetCreatedCallbacks[deletedMetadata.type](deletedMetadata);
					}
				}

				for (AssetHandle handle : selectedItems)
				{
					if (myDirectories.find(handle) != myDirectories.end())
					{
						DeleteDirectory(myDirectories[handle]);
					}
				}

				SelectionManager::DeselectAll(SelectionContext::ContentBrowser);
				Refresh();
				
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor(3);
		
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
		
			if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f)))
			{
				ImGui::CloseCurrentPopup();
			}
		
			UI::Fonts::PopFont();
			ImGui::EndPopup();
		}
		UI::Fonts::PopFont();
	}

	void ContentBrowserPanel::RenderNewScriptDialogue()
	{
		UI::Fonts::PushFont("Bold");
		if (ImGui::BeginPopupModal(staticNewScriptModalName, NULL, ImGuiWindowFlags_AlwaysAutoResize))
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
			ImGui::Separator();
			UI::Fonts::PopFont();

			ImGui::SetNextItemWidth(248.0f);
			ImGui::InputTextWithHint("##new_script_namespace", "Namespace...", newScriptNamespaceBuffer, 256);
			ImGui::SetNextItemWidth(248.0f);
			ImGui::InputTextWithHint("##new_script_name", "Script Name...", newScriptNameBuffer, 256);

			ImGui::Separator();

			const bool fileAlreadyExists = FileSystem::Exists(Project::GetAssetDirectory() / myCurrentDirectory->filePath / (std::string(newScriptNameBuffer) + ".cs"));
			ImGui::BeginDisabled(strlen(newScriptNameBuffer) == 0 || fileAlreadyExists);
			if (ImGui::Button("Create", ImVec2(120.0f, 0.0f)))
			{
				CreateAsset<ScriptFileAsset>(std::string(newScriptNameBuffer) + ".cs", namespaceString.c_str(), newScriptNameBuffer);

				memset(newScriptNamespaceBuffer, 0, 255);
				memset(newScriptNameBuffer, 0, 255);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndDisabled();

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f)))
			{
				memset(newScriptNameBuffer, 0, 255);
				ImGui::CloseCurrentPopup();
			}

			UI::Fonts::PopFont();
			ImGui::EndPopup();
		}
		UI::Fonts::PopFont();
	}

	void ContentBrowserPanel::DeleteDirectory(std::shared_ptr<DirectoryInfo>& aDirectory, bool aRemoveFromParent)
	{
		if (aDirectory->parent && aRemoveFromParent)
		{
			auto& childList = aDirectory->parent->subDirectories;
			childList.erase(childList.find(aDirectory->handle));
		}

		for (auto& [handle, subdir] : aDirectory->subDirectories)
		{
			DeleteDirectory(subdir, false);
		}

		aDirectory->subDirectories.clear();
		aDirectory->assets.clear();

		myDirectories.erase(myDirectories.find(aDirectory->handle));
	}

	void ContentBrowserPanel::UpdateDropArea(const std::shared_ptr<DirectoryInfo>& aTarget)
	{
		if (aTarget->handle != myCurrentDirectory->handle && ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset_payload");

			if (payload)
			{
				uint32_t count = payload->DataSize / sizeof(AssetHandle);

				for (uint32_t i = 0; i < count; i++)
				{
					AssetHandle assetHandle = *(((AssetHandle*)payload->Data) + i);
					size_t index = myCurrentItems.FindItem(assetHandle);
					if (index != ContentBrowserItemList::InvalidItem)
					{
						if (myCurrentItems[index]->Move(aTarget->filePath))
						{
							myCurrentItems.Erase(assetHandle);
							myShouldRefresh = true;
						}
					}
				}
			}

			ImGui::EndDragDropTarget();
		}
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
