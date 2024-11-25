#include "ContentBrowserPanel.h"
#include <fstream>
#include <CommonUtilities/Math/CommonMath.hpp>
#include <CommonUtilities/StringUtils.h>
#include <Epoch/ImGui/ImGui.h>
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
#include <Epoch/Editor/FontAwesome.h>
#include "Aeon/EditorResources.h"

namespace Epoch
{
	void ContentBrowserPanel::Init()
	{
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

	static bool staticOpenNewScriptPopup = false;

	void ContentBrowserPanel::OnImGuiRender(bool& aIsOpen)
	{
		EPOCH_PROFILE_FUNC();

		myAnEntryHovered = false;
		myDeleteSelected = false;

		ImGui::Begin(std::format("{}  Content Browser", EP_ICON_INBOX).c_str());

		if (myBaseDirectory.empty())
		{
			ImGui::End();
			return;
		}

		if (myCurrentDirectory != myBaseDirectory)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_MouseX1))
			{
				myCurrentDirectory = myCurrentDirectory.parent_path();
				myRefresh = true;
			}
		}

		// Content browser bar
		{
			// Search
			{
				static std::string searchString;

				ImGui::SetNextItemWidth(250);
				ImGui::InputTextWithHint("##AssetSearch", "Search...", &searchString);
			}

			// Directory buttons
			{
				ImGui::SameLine();

				std::filesystem::path tempPath = myCurrentDirectory;
				std::vector<std::filesystem::path> paths;
				while (tempPath != myBaseDirectory)
				{
					paths.push_back(tempPath);
					tempPath = tempPath.parent_path();
				}
				paths.push_back(myBaseDirectory);

				UI::Fonts::PushFont("Bold");
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.65f, 1));
				for (int i = (int)paths.size() - 1; i >= 0; i--)
				{
					ImGui::SameLine();
					if (ImGui::Button(paths[i].stem().string().c_str()))
					{
						myCurrentDirectory = paths[i];
						myRefresh = true;
					}

					if (i != 0)
					{
						ImGui::SameLine();
						ImGui::Text("/");
					}
				}
				ImGui::PopStyleColor(3);
				UI::Fonts::PopFont();
			}

			// Size slider
			{
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200.0f);
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 200.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				ImGui::SliderFloat("##Thumbnail Size", &EditorSettings::Get().contentBrowserThumbnailSize, myThumbnailMinSize, myThumbnailMaxSize, NULL, ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);
				ImGui::PopStyleColor();
			}
		}

		ImGui::Separator();
		ImGui::BeginChild("ContentView", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30 });

		ImVec2 contentViewCursorStartPos = ImGui::GetCursorPos();

		ContentView();

		ImGui::SetCursorPos(contentViewCursorStartPos);
		ImGui::Dummy(ImGui::GetContentRegionAvail());

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

			if (payload)
			{
				size_t count = payload->DataSize / sizeof(UUID);

				for (size_t i = 0; i < count; i++)
				{
					UUID entityID = *(((UUID*)payload->Data) + i);
					Entity entity = mySceneContext->GetEntityWithUUID(entityID);

					if (!entity)
					{
						LOG_ERROR("Failed to find entity with ID {} in current scene context!", entityID);
						continue;
					}

					std::shared_ptr<Prefab> prefab = CreateAsset<Prefab>(entity.GetName() + ".prefab");
					prefab->Create(entity);
					AssetImporter::Serialize(prefab);
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::OpenPopupOnItemClick("Content Popup", ImGuiPopupFlags_MouseButtonRight);
		ContentViewPopup();

		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		{
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			{
				float scroll = ImGui::GetIO().MouseWheel;
				if (scroll != 0.0f)
				{
					EditorSettings::Get().contentBrowserThumbnailSize += scroll * 5.0f;
					EditorSettings::Get().contentBrowserThumbnailSize = CU::Math::Clamp(EditorSettings::Get().contentBrowserThumbnailSize, myThumbnailMinSize, myThumbnailMaxSize);
				}
			}
			else if (!myAnEntryHovered)
			{
				if (ImGui::IsKeyPressed(ImGuiKey_MouseLeft) || ImGui::IsKeyPressed(ImGuiKey_MouseRight))
				{
					if (mySelectionCount > 0)
					{
						ClearSelection();
					}

					myRenaming = false;
				}
			}
		}

		ImGui::EndChild();

		ImGui::Separator();

		if (mySelectionCount == 1)
		{
			for (const DirectoryEntry& entry : myDirectoryEntries)
			{
				if (!entry.isSelected)
				{
					continue;
				}

				std::string filePath = "  " + std::filesystem::relative(entry.entry.path(), myBaseDirectory.parent_path()).string();
				ImGui::Text(filePath.c_str());
			}
		};

		//if (mySelectionCount > 0 && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
		//{
		//	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		//	{
		//		myDeleteSelected = true;
		//	}
		//	else if (ImGui::IsKeyPressed(ImGuiKey_D) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
		//	{
		//		DuplicateSelected();
		//	}
		//}

		if (myDeleteSelected)
		{
			ImGui::OpenPopup(" Delete selected asset(s)?");
		}
		DeleteModal();

		if (staticOpenNewScriptPopup)
		{
			ImGui::OpenPopup("New Script");
			staticOpenNewScriptPopup = false;
		}
		RenderNewScriptDialogue();

		ImGui::End();

		if (myRefresh)
		{
			myRefresh = false;
			Refresh();
		}


		//TEMP
		{
			EPOCH_PROFILE_SCOPE("ContentBrowserPanel::OnImGuiRender::TEMP::UpdateSelectedAssets()");

			SelectionManager::DeselectAll(SelectionContext::Asset);

			for (auto entry : myDirectoryEntries)
			{
				if (entry.isSelected)
				{
					auto assetManager = Project::GetEditorAssetManager();
					auto assetID = assetManager->GetAssetHandleFromFilePath(entry.entry.path());
					if (assetManager->IsAssetHandleValid(assetID))
					{
						SelectionManager::Select(SelectionContext::Asset, assetID);
					}
				}
			}
		}
	}

	void ContentBrowserPanel::OnProjectChanged(const std::shared_ptr<Project>& aProject)
	{
		myBaseDirectory = aProject->GetAssetDirectory();;
		myCurrentDirectory = myBaseDirectory;

		myRefresh = true;
	}

	void ContentBrowserPanel::DeleteModal()
	{
		UI::Fonts::PushFont("Bold");
		if (ImGui::BeginPopupModal(" Delete selected asset(s)?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			UI::Fonts::PushFont("Regular");

			float width = ImGui::GetContentRegionAvail().x;
			ImGui::BeginChild("Files to delete", ImVec2(width, 70.0f));
			for (const DirectoryEntry& entry : myDirectoryEntries)
			{
				if (!entry.isSelected)
				{
					continue;
				}

				std::string filePath = "  " + std::filesystem::relative(entry.entry.path(), myBaseDirectory.parent_path()).string();
				ImGui::Text(filePath.c_str());
			}
			ImGui::EndChild();

			UI::Spacing();
			ImGui::Separator();
			UI::Fonts::PushFont("Bold");
			ImGui::Text(" You cannot undo this action.");
			UI::Fonts::PopFont();
			UI::Spacing();
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6196f, 0.1373f, 0.1373f, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7196f, 0.2373f, 0.2373f, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5196f, 0.0373f, 0.0373f, 1));
			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				DeleteSelected();
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor(3);

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			UI::Fonts::PopFont();
			ImGui::EndPopup();
		}
		UI::Fonts::PopFont();
	}

	void ContentBrowserPanel::DeleteSelected()
	{
		auto assetManager = Project::GetEditorAssetManager();

		for (const DirectoryEntry& entry : myDirectoryEntries)
		{
			if (!entry.isSelected)
			{
				continue;
			}

			if (entry.entry.is_directory())
			{
				std::filesystem::remove_all(entry.entry.path());
			}
			else
			{
				const auto& metadata = assetManager->GetMetadata(entry.entry.path());
				assetManager->OnAssetDeleted(metadata.handle);
				std::filesystem::remove(entry.entry.path());
			}

			const auto find = staticAssetExtensionMap.find(entry.entry.path().extension().string());

			if (find != staticAssetExtensionMap.end())
			{
				auto assetType = find->second;
				if (myAssetDeletedCallbacks.find(assetType) != myAssetDeletedCallbacks.end())
				{
					auto assetManager = Project::GetEditorAssetManager();
					const AssetMetadata& metadata = assetManager->GetMetadata(assetManager->GetAssetHandleFromFilePath(entry.entry.path()));
					myAssetDeletedCallbacks[assetType](metadata);
				}
			}
		}

		myRefresh = true;
	}

	void ContentBrowserPanel::DuplicateSelected()
	{
		//for (const DirectoryEntry& entry : myDirectoryEntries)
		//{
		//	if (!entry.isSelected)
		//	{
		//		continue;
		//	}
		//
		//	std::string copyName = GetNewName(entry.entry.path().stem().string()) + entry.entry.path().extension().string();
		//
		//	if (entry.entry.is_directory())
		//	{
		//		std::filesystem::copy(entry.entry.path(), myCurrentDirectory / copyName);
		//	}
		//	else
		//	{
		//		FileSystem::CopyFile(entry.entry.path(), myCurrentDirectory);
		//	}
		//}
		//
		//myRefresh = true;
	}

	void ContentBrowserPanel::DuplicateSelected(const std::filesystem::path& aNewPath)
	{
		for (const DirectoryEntry& entry : myDirectoryEntries)
		{
			if (!entry.isSelected)
			{
				continue;
			}

			std::string copyName = GetNewName(entry.entry.path().stem().string()) + entry.entry.path().extension().string();

			if (entry.entry.is_directory())
			{
				std::filesystem::copy(entry.entry.path(), aNewPath / copyName);
			}
			else
			{
				std::filesystem::copy_file(entry.entry.path(), aNewPath / copyName);
			}
		}
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

			const bool fileAlreadyExists = FileSystem::Exists(Project::GetAssetDirectory() / myCurrentDirectory / (std::string(newScriptNameBuffer) + ".cs"));
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

	void ContentBrowserPanel::ClearSelection()
	{
		for (DirectoryEntry& entry : myDirectoryEntries)
		{
			entry.isSelected = false;
		}

		mySelectionCount = 0;
	}

	void ContentBrowserPanel::Refresh()
	{
		EPOCH_PROFILE_FUNC();

		if (!std::filesystem::directory_entry(myCurrentDirectory).exists())
		{
			LOG_ERROR("The directory you were browsing no longer exists!");
			myCurrentDirectory = myBaseDirectory;
		}

		ClearSelection();
		myDirectoryEntries.clear();

		for (const std::filesystem::directory_entry& directoryEntry : std::filesystem::directory_iterator(myCurrentDirectory))
		{
			myDirectoryEntries.push_back(DirectoryEntry(directoryEntry));
		}
	}

	void ContentBrowserPanel::RenderDirectoryHierarchy(const std::filesystem::directory_entry& aEntry, bool aDefaultOpen)
	{
		bool isLeaf = true;
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(aEntry.path()))
		{
			if (entry.is_directory())
			{
				isLeaf = false;
			}
		}

		ImGuiTreeNodeFlags flags = (aDefaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0) | ImGuiTreeNodeFlags_SpanAvailWidth;
		isLeaf ? flags |= ImGuiTreeNodeFlags_Leaf : flags |= ImGuiTreeNodeFlags_OpenOnArrow;

		bool open = ImGui::TreeNodeEx(aEntry.path().string().c_str(), flags, aEntry.path().stem().string().c_str());

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			myCurrentDirectory = aEntry.path();
			myRefresh = true;
		}

		if (open)
		{
			for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(aEntry.path()))
			{
				if (entry.is_directory())
				{
					RenderDirectoryHierarchy(entry);
				}
			}

			ImGui::TreePop();
		}
	}

	void ContentBrowserPanel::ContentView()
	{
		//ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable
		//	| ImGuiTableFlags_SizingFixedFit
		//	| ImGuiTableFlags_BordersInnerV;
		//UI::PushID();
		//if (ImGui::BeginTable("ContentBrowserTable", 2, tableFlags))
		{
			//ImGui::TableSetupColumn("Outliner", 0, 300.0f);
			//ImGui::TableSetupColumn("Directory Structure", ImGuiTableColumnFlags_WidthStretch);
			//
			//ImGui::TableNextRow();
			//ImGui::TableSetColumnIndex(0);
			//
			//ImGui::BeginChild("##folders_common");
			//{
			//	RenderDirectoryHierarchy(std::filesystem::directory_entry(myBaseDirectory), true);
			//}
			//ImGui::EndChild();
			//
			//ImGui::TableSetColumnIndex(1);
			//
			//ImGui::BeginChild("##directory_structure", ImGui::GetContentRegionAvail());
			{
				const float thumbnailSize = EditorSettings::Get().contentBrowserThumbnailSize;

				float cellSize = thumbnailSize + myPadding;

				float panelWidth = ImGui::GetContentRegionAvail().x;
				int columnCount = CU::Math::Max(static_cast<int>(panelWidth / cellSize), 1);

				ImGui::Columns(columnCount, 0, false);

				auto icon = EditorResources::OtherIcon;
				for (DirectoryEntry& entry : myDirectoryEntries)
				{
					bool entryHovered = false;
					const std::filesystem::path& entryPath = entry.entry.path();

					ImGui::PushID(entryPath.filename().string().c_str());
					if (myAssetIconMap.find(entryPath.extension().string()) != myAssetIconMap.end())
					{
						icon = myAssetIconMap[entryPath.extension().string()];
					}
					else
					{
						icon = EditorResources::OtherIcon;
					}

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					if (icon)
					{
						ImGui::ImageButton((ImTextureID)icon->GetView(), { thumbnailSize, thumbnailSize });
					}
					else
					{
						ImGui::ImageButton(nullptr, { thumbnailSize, thumbnailSize });
					}
					ImGui::PopStyleColor();

					entryHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
					if (!myAnEntryHovered)
					{
						myAnEntryHovered = entryHovered;
					}

					bool doubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

					if (entryHovered && doubleClicked)
					{
						if (entry.entry.is_directory())
						{
							myCurrentDirectory /= entryPath.filename();
							myRefresh = true;
						}
						else
						{
							const auto find = staticAssetExtensionMap.find(entry.entry.path().extension().string());

							if (find != staticAssetExtensionMap.end())
							{
								auto assetType = find->second;
								if (myItemActivationCallbacks.find(assetType) != myItemActivationCallbacks.end())
								{
									auto assetManager = Project::GetEditorAssetManager();
									const AssetMetadata& metadata = assetManager->GetMetadata(assetManager->GetAssetHandleFromFilePath(entry.entry.path()));
									myItemActivationCallbacks[assetType](metadata);
								}
							}
						}
					}

					if (ImGui::BeginDragDropSource())
					{
						auto assetManager = Project::GetEditorAssetManager();
						AssetHandle assetID = assetManager->GetAssetHandleFromFilePath(entry.entry.path());
						ImGui::SetDragDropPayload("asset_payload", &assetID, sizeof(AssetHandle));
						ImGui::Text(entry.entry.path().stem().string().c_str());
						ImGui::EndDragDropSource();
					}

					bool renamingEntry = myEntryToRename == entry.entry;
					static char renameBuffer[256];

					if (myRenameTrigger && renamingEntry)
					{
						myRenameTrigger = false;
						myRenaming = true;

						strcpy_s(renameBuffer, entryPath.stem().string().c_str());
						ImGui::SetKeyboardFocusHere();
					}

					if (myRenaming && renamingEntry)
					{
						if (ImGui::IsKeyPressed(ImGuiKey_Escape))
						{
							myRenaming = false;
						}

						ImGui::SetNextItemWidth(thumbnailSize);
						ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
						if (ImGui::InputText("##NewName", renameBuffer, IM_ARRAYSIZE(renameBuffer), flags))
						{
							if (!std::string(renameBuffer).empty())
							{
								auto assetManager = Project::GetEditorAssetManager();
								std::shared_ptr<Asset> asset = assetManager->GetAsset(assetManager->GetAssetHandleFromFilePath(entryPath));

								std::string newName = FileSystem::GetUniqueFileName(std::string(renameBuffer) + entryPath.extension().string()).string();
								const std::filesystem::path newPath = myCurrentDirectory / newName;

								std::error_code error;
								std::filesystem::rename(entryPath, newPath, error);

								if (error.value())
								{
									LOG_ERROR("{}", error.message());
								}
								else if (asset)
								{
									const std::filesystem::path currentPath = assetManager->GetFileSystemPath(asset->GetHandle());
									assetManager->OnAssetRenamed(asset->GetHandle(), newPath);
									if (asset->GetAssetType() == AssetType::Scene && mySceneContext->GetName() == currentPath.stem().string())
									{
										if (myCurrentSceneRenamedCallback)
										{
											myCurrentSceneRenamedCallback(assetManager->GetMetadata(asset->GetHandle()));
										}
										else
										{
											LOG_WARNING("No callback for when open scene gets renamed.");
										}
									}
								}

								myRefresh = true;
								myRenaming = false;
							}
						}
					}
					else
					{
						std::string name = entryPath.stem().string();
						float textWidth = ImGui::CalcTextSize(name.c_str()).x;
						int charCount = (int)name.length();

						if (textWidth > thumbnailSize)
						{
							while (textWidth * 0.5f > thumbnailSize)
							{
								name.erase((int)(charCount * 0.5f));
								textWidth = ImGui::CalcTextSize(name.c_str()).x;
								charCount = (int)name.length();
							}

							while (textWidth > thumbnailSize)
							{
								name.pop_back();
								textWidth = ImGui::CalcTextSize(name.c_str()).x;
								charCount--;
							}

							if (charCount > 3)
							{
								name.erase(charCount - 3);
								name += "...";
								textWidth = ImGui::CalcTextSize(name.c_str()).x;
							}
						}

						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (thumbnailSize - textWidth) * 0.5f);

						if (entry.isSelected)
						{
							ImGui::PushStyleColor(ImGuiCol_Text, Colors::Theme::blue);
						}
						ImGui::Text(name.c_str());
						if (entry.isSelected)
						{
							ImGui::PopStyleColor();
						}

						if (ImGui::IsItemHovered() && doubleClicked)
						{
							myRenameTrigger = true;
							ClearSelection();
							mySelectionCount++;
							myEntryToRename = entry.entry;
						}
					}

					if (entryHovered)
					{
						if (ImGui::IsKeyPressed(ImGuiKey_MouseLeft) || ImGui::IsKeyPressed(ImGuiKey_MouseRight))
						{
							if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
							{
								if (entry.isSelected)
								{
									entry.isSelected = false;
									mySelectionCount--;
								}
								else
								{
									entry.isSelected = true;
									mySelectionCount++;
								}
							}
							else
							{
								ClearSelection();
								entry.isSelected = true;
								mySelectionCount++;
							}

							myRenaming = false;
						}
					}

					ImGui::PopID();
					ImGui::NextColumn();
				}

				ImGui::Columns(1);
			}
			//ImGui::EndChild();

			//ImGui::EndTable();
		}

		//UI::PopID();
	}

	void ContentBrowserPanel::ContentViewPopup()
	{
		bool entrySelected = mySelectionCount == 1;
		int entryIndex = 0;

		if (entrySelected)
		{
			for (int i = 0; i < (int)myDirectoryEntries.size(); i++)
			{
				if (myDirectoryEntries[i].isSelected)
				{
					entryIndex = i;
					break;
				}
			}
		}

		if (ImGui::BeginPopup("Content Popup", ImGuiWindowFlags_NoMove))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Folder"))
				{
					auto path = FileSystem::GetUniqueFileName(myCurrentDirectory / "New Folder");
					std::filesystem::create_directory(path);

					myRefresh = true;
					myRenameTrigger = true;
					myEntryToRename = std::filesystem::directory_entry(path);
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Scene"))
				{
					auto path = FileSystem::GetUniqueFileName(myCurrentDirectory / "New Scene.epoch");
					CreateAsset<Scene>(path.filename().string());

					myRefresh = true;
					myRenameTrigger = true;
					myEntryToRename = std::filesystem::directory_entry(path);
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Material"))
				{
					auto path = FileSystem::GetUniqueFileName(myCurrentDirectory / "New Material.mat");
					CreateAsset<Material>(path.filename().string());

					myRefresh = true;
					myRenameTrigger = true;
					myEntryToRename = std::filesystem::directory_entry(path);
				}
				
				ImGui::Separator();

				if (ImGui::MenuItem("Script"))
				{
					staticOpenNewScriptPopup = true;
				}

				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Show in Explorer"))
			{
				if (entrySelected)
				{
					FileSystem::ShowFileInExplorer(myDirectoryEntries[entryIndex].entry.path());
				}
				else
				{
					FileSystem::OpenDirectoryInExplorer(myCurrentDirectory);
				}
			}

			if (ImGui::MenuItem("Open Externally", 0, false, entrySelected))
			{
				FileSystem::OpenExternally(myDirectoryEntries[entryIndex].entry.path());
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Import..."))
			{
				std::vector<std::filesystem::path> assetPaths = FileSystem::OpenFileDialogMultiple({ { "Asset",	"png,jpg,jpeg,hdr,fbx,obj,gltf,glb,ttf" } }, myCurrentDirectory.string().c_str());

				if (assetPaths.size() > 0)
				{
					auto assetManager = Project::GetEditorAssetManager();
					for (const auto& path : assetPaths)
					{
						if (FileSystem::Exists(myCurrentDirectory / path.filename()))
						{
							assetManager->ImportAsset(myCurrentDirectory / path.filename());
						}
						else if (path.extension() == ".gltf")
						{
							auto binPath = CU::RemoveExtension(path.string()) + ".bin";
							if (FileSystem::CopyFile(path, myCurrentDirectory) && FileSystem::CopyFile(binPath, myCurrentDirectory))
							{
								assetManager->ImportAsset(myCurrentDirectory / path.filename());
							}
						}
						else
						{
							if (FileSystem::CopyFile(path, myCurrentDirectory))
							{
								assetManager->ImportAsset(myCurrentDirectory / path.filename());
							}
						}
					}

					Refresh();
				}
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Reload", 0, false, entrySelected))
			{
				auto assetManager = Project::GetEditorAssetManager();
				AssetHandle asset = assetManager->GetAssetHandleFromFilePath(myDirectoryEntries[entryIndex].entry.path());
				assetManager->ReloadData(asset);
			}

			if (ImGui::MenuItem("Delete", 0, false, entrySelected))
			{
				myDeleteSelected = true;
			}

			if (ImGui::MenuItem("Duplicate", 0, false, entrySelected))
			{
				DuplicateSelected();
			}

			if (ImGui::MenuItem("Rename", 0, false, entrySelected))
			{
				myRenameTrigger = true;
				myEntryToRename = myDirectoryEntries[entryIndex].entry;
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Refresh"))
			{
				myRefresh = true;
			}

			ImGui::EndPopup();
		}
	}

	std::string ContentBrowserPanel::GetNewName(const std::string& aName) const
	{
		const std::string name = GetNameWithoutIndex(aName);
		std::string newName = name;

		int index = 1;
		if (std::filesystem::exists(myCurrentDirectory / newName))
		{
			newName = name + " " + std::to_string(index);

			while (std::filesystem::exists(myCurrentDirectory / newName))
			{
				index++;
				newName = name + " " + std::to_string(index);
			}
		}

		return newName;
	}

	std::string ContentBrowserPanel::GetNameWithoutIndex(const std::string& aName) const
	{
		const std::size_t found = aName.find_last_of(" ");
		const std::string name = aName.substr(0, found);
		const std::string index = aName.substr(found + 1);

		char* p;
		long converted = strtol(index.c_str(), &p, 10);
		converted;
		bool indexed = !*p;

		if (indexed)
		{
			return name;
		}
		else
		{
			return aName;
		}
	}
}
