#pragma once
#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <Epoch/Scene/Scene.h>
#include <Epoch/Utils/FileSystem.h>
#include <Epoch/Rendering/Texture.h>
#include <Epoch/Assets/AssetMetadata.h>
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel() = default;
		~ContentBrowserPanel() = default;

		void Init();

		void OnImGuiRender(bool& aIsOpen) override;

		void OnEvent(Event& aEvent) override;

		void OnProjectChanged(const std::shared_ptr<Project>& aProject) override;
		void OnSceneChanged(const std::shared_ptr<Scene>& aScene) override { mySceneContext = aScene; }

		void RegisterItemActivateCallbackForType(AssetType aType, const std::function<void(const AssetMetadata&)>& aCallback) { myItemActivationCallbacks[aType] = aCallback; }
		void RegisterNewAssetCreatedCallbackForType(AssetType aType, const std::function<void(const AssetMetadata&)>& aCallback) { myNewAssetCreatedCallbacks[aType] = aCallback; }
		void RegisterAssetDeletedCallbackForType(AssetType aType, const std::function<void(const AssetMetadata&)>& aCallback) { myAssetDeletedCallbacks[aType] = aCallback; }
		void RegistryCurrentSceneRenamedCallback(const std::function<void(const AssetMetadata&)>& aCallback) { myCurrentSceneRenamedCallback = aCallback; }

	private:
		void DeleteModal();
		void DeleteSelected();
		void DuplicateSelected();
		void DuplicateSelected(const std::filesystem::path& aNewPath);
		
		void RenderNewScriptDialogue();

		void ClearSelection();
		void Refresh();

		void RenderDirectoryHierarchy(const std::filesystem::directory_entry& aEntry, bool aDefaultOpen = false);
		void ContentView();
		void ContentViewPopup();

		bool ImportAssets(const std::vector<std::string>& aPaths);
		void ImportAssets(const std::vector<std::filesystem::path>& aPaths);

		std::string GetNewName(const std::string& aName) const;
		std::string GetNameWithoutIndex(const std::string& aName) const;

	private:
		template<typename T, typename... Args>
		std::shared_ptr<T> CreateAsset(const std::string& aFilename, Args&&... aArgs)
		{
			return CreateAssetInDirectory<T>(aFilename, myCurrentDirectory, std::forward<Args>(aArgs)...);
		}

		template<typename T, typename... Args>
		std::shared_ptr<T> CreateAssetInDirectory(const std::string& aFilename, const std::filesystem::path& aDirectory, Args&&... aArgs)
		{
			auto filepath = FileSystem::GetUniqueFileName(Project::GetAssetDirectory() / aDirectory / aFilename);

			std::shared_ptr<T> asset = Project::GetEditorAssetManager()->CreateNewAsset<T>(filepath.filename().string(), aDirectory.string(), std::forward<Args>(aArgs)...);
			if (!asset)
			{
				return nullptr;
			}

			if (myNewAssetCreatedCallbacks.find(T::GetStaticType()) != myNewAssetCreatedCallbacks.end())
			{
				const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(asset->GetHandle());
				myNewAssetCreatedCallbacks[T::GetStaticType()](metadata);
			}

			Refresh();

			return asset;
		}

	private:
		std::filesystem::path myBaseDirectory;
		std::filesystem::path myCurrentDirectory;

		std::shared_ptr<Scene> mySceneContext;

		bool myRefresh = false;
		bool myDeleteSelected = false;
		bool myAnEntryHovered = false;
		bool myRenameTrigger = false;
		bool myRenaming = false;

		float myPadding = 20.0f;
		float myThumbnailMinSize = 64.0f;
		float myThumbnailMaxSize = 256.0f;

		int mySelectionCount = 0;

		std::filesystem::directory_entry myEntryToRename;

		struct DirectoryEntry
		{
			std::filesystem::directory_entry entry;
			bool isSelected = false;

			DirectoryEntry(const std::filesystem::directory_entry& aEntry) : entry(aEntry) {}
		};
		std::vector<DirectoryEntry> myDirectoryEntries;

		std::unordered_map<std::string, std::shared_ptr<Texture2D>> myAssetIconMap;

		std::unordered_map<AssetType, std::function<void(const AssetMetadata&)>> myItemActivationCallbacks;
		std::unordered_map<AssetType, std::function<void(const AssetMetadata&)>> myNewAssetCreatedCallbacks;
		std::unordered_map<AssetType, std::function<void(const AssetMetadata&)>> myAssetDeletedCallbacks;
		std::function<void(const AssetMetadata&)> myCurrentSceneRenamedCallback;
	};
}
