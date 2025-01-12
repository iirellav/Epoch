#pragma once
#include <functional>
#include <Epoch/Scene/Scene.h>
#include <Epoch/Utils/FileSystem.h>
#include <Epoch/Rendering/Texture.h>
#include <Epoch/Assets/AssetMetadata.h>
#include <Epoch/Editor/EditorPanel.h>
#include <Epoch/Core/Events/MouseEvent.h>
#include <Epoch/Core/Events/KeyEvent.h>
#include <Epoch/Core/Events/EditorEvents.h>
#include "ContentBrowser/SelectionStack.h"
#include "ContentBrowser/ContentBrowserItem.h"

#define MAX_INPUT_BUFFER_LENGTH 128

namespace Epoch
{
	struct ContentBrowserItemList
	{
		static constexpr size_t InvalidItem = std::numeric_limits<size_t>::max();

		std::vector<std::shared_ptr<ContentBrowserItem>> items;

		std::vector<std::shared_ptr<ContentBrowserItem>>::iterator begin() { return items.begin(); }
		std::vector<std::shared_ptr<ContentBrowserItem>>::iterator end() { return items.end(); }
		std::vector<std::shared_ptr<ContentBrowserItem>>::const_iterator begin() const { return items.begin(); }
		std::vector<std::shared_ptr<ContentBrowserItem>>::const_iterator end() const { return items.end(); }

		std::shared_ptr<ContentBrowserItem>& operator[](size_t index) { return items[index]; }
		const std::shared_ptr<ContentBrowserItem>& operator[](size_t index) const { return items[index]; }

		ContentBrowserItemList() = default;

		ContentBrowserItemList(const ContentBrowserItemList& aOther) : items(aOther.items) {}

		ContentBrowserItemList& operator=(const ContentBrowserItemList& aOther)
		{
			items = aOther.items;
			return *this;
		}

		void Clear()
		{
			std::scoped_lock<std::mutex> lock(myMutex);
			items.clear();
		}

		void Erase(AssetHandle aHandle)
		{
			size_t index = FindItem(aHandle);
			if (index == InvalidItem)
			{
				return;
			}

			std::scoped_lock<std::mutex> lock(myMutex);
			auto it = items.begin() + index;
			items.erase(it);
		}

		size_t FindItem(AssetHandle aHandle)
		{
			if (items.empty())
			{
				return InvalidItem;
			}

			std::scoped_lock<std::mutex> lock(myMutex);
			for (size_t i = 0; i < items.size(); i++)
			{
				if (items[i]->GetID() == aHandle)
				{
					return i;
				}
			}

			return InvalidItem;
		}

	private:
		std::mutex myMutex;
	};

	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		static ContentBrowserPanel& Get() { return *staticInstance; }

		void OnImGuiRender(bool& aIsOpen) override;

		void OnEvent(Event& aEvent) override;

		void OnProjectChanged(const std::shared_ptr<Project>& aProject) override;
		void OnSceneChanged(const std::shared_ptr<Scene>& aScene) override { mySceneContext = aScene; }

		std::shared_ptr<DirectoryInfo> GetDirectory(const std::filesystem::path& aFilepath) const;
		ContentBrowserItemList& GetCurrentItems() { return myCurrentItems; }

		void RegisterItemActivateCallbackForType(AssetType aType, const std::function<void(const AssetMetadata&)>& aCallback) { myItemActivationCallbacks[aType] = aCallback; }
		void RegisterNewAssetCreatedCallbackForType(AssetType aType, const std::function<void(const AssetMetadata&)>& aCallback) { myNewAssetCreatedCallbacks[aType] = aCallback; }
		void RegisterAssetDeletedCallbackForType(AssetType aType, const std::function<void(const AssetMetadata&)>& aCallback) { myAssetDeletedCallbacks[aType] = aCallback; }
		void RegistryCurrentSceneRenamedCallback(const std::function<void(const AssetMetadata&)>& aCallback) { myCurrentSceneRenamedCallback = aCallback; }

	private:
		void UpdateInput();

		bool OnKeyPressedEvent(KeyPressedEvent& aEvent);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& aEvent);
		bool OnFileDrop(EditorFileDroppedEvent& aEvent);

		void Import(const std::vector<std::filesystem::path>& aPaths);

		void Refresh();
		AssetHandle ProcessDirectory(const std::filesystem::path& aDirectoryPath, const std::shared_ptr<DirectoryInfo>& aParent);

		void ChangeDirectory(std::shared_ptr<DirectoryInfo>& aDirectory);
		void OnBrowseBack();
		void OnBrowseForward();

		void RenderDirectoryHierarchy(std::shared_ptr<DirectoryInfo>& aDirectory, bool aDefaultOpen = false);
		void RenderTopBar(float aHeight);
		void RenderItems();
		void RenderBottomBar(float aHeight);

		void ClearSelections();

		void PasteCopiedAssets();

		void RenderDeleteDialogue();
		void RenderNewScriptDialogue();

		void DeleteDirectory(std::shared_ptr<DirectoryInfo>& aDirectory, bool aRemoveFromParent = true);

		void UpdateDropArea(const std::shared_ptr<DirectoryInfo>& aTarget);

		void SortItemList();

		ContentBrowserItemList Search(const std::string& aQuery, const std::shared_ptr<DirectoryInfo>& aDirectoryInfo);

	private:
		template<typename T, typename... Args>
		std::shared_ptr<T> CreateAsset(const std::string& aFilename, Args&&... aArgs)
		{
			return CreateAssetInDirectory<T>(aFilename, myCurrentDirectory, std::forward<Args>(aArgs)...);
		}

		template<typename T, typename... Args>
		std::shared_ptr<T> CreateAssetInDirectory(const std::string& aFilename, std::shared_ptr<DirectoryInfo>& aDirectory, Args&&... aArgs)
		{
			std::filesystem::path filepath = FileSystem::GetUniqueFileName(Project::GetAssetDirectory() / aDirectory->filePath / aFilename);

			std::shared_ptr<T> asset = Project::GetEditorAssetManager()->CreateNewAsset<T>(filepath.filename().string(), aDirectory->filePath.string(), std::forward<Args>(aArgs)...);
			if (!asset)
			{
				return nullptr;
			}

			aDirectory->assets.push_back(asset->GetHandle());

			if (myNewAssetCreatedCallbacks.find(T::GetStaticType()) != myNewAssetCreatedCallbacks.end())
			{
				const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(asset->GetHandle());
				myNewAssetCreatedCallbacks[T::GetStaticType()](metadata);
			}

			Refresh();

			return asset;
		}

	private:
		static inline ContentBrowserPanel* staticInstance;

		std::shared_ptr<Scene> mySceneContext;

		ContentBrowserItemList myCurrentItems;

		std::shared_ptr<DirectoryInfo> myCurrentDirectory;
		std::shared_ptr<DirectoryInfo> myBaseDirectory;
		std::shared_ptr<DirectoryInfo> myNextDirectory, myPreviousDirectory;

		std::unordered_map<AssetHandle, std::shared_ptr<DirectoryInfo>> myDirectories;

		bool myIsAnyItemHovered = false;
		bool myShouldRefresh = false;

		SelectionStack myCopiedAssets;

		std::unordered_map<std::string, std::shared_ptr<Texture2D>> myAssetIconMap;

		std::unordered_map<AssetType, std::function<void(const AssetMetadata&)>> myItemActivationCallbacks;
		std::unordered_map<AssetType, std::function<void(const AssetMetadata&)>> myNewAssetCreatedCallbacks;
		std::unordered_map<AssetType, std::function<void(const AssetMetadata&)>> myAssetDeletedCallbacks;
		std::function<void(const AssetMetadata&)> myCurrentSceneRenamedCallback;

		char mySearchBuffer[MAX_INPUT_BUFFER_LENGTH];

		std::vector<std::shared_ptr<DirectoryInfo>> myBreadCrumbData;
		bool myUpdateNavigationPath = false;

		bool myIsContentBrowserHovered = false;
		bool myIsContentBrowserFocused = false;
	};
}
