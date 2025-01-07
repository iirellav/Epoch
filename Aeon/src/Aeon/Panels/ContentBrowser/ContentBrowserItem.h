#pragma once
#include <map>
#include <filesystem>
#include <Epoch/Core/Base.h>
#include <Epoch/Assets/AssetMetadata.h>
#include <Epoch/Rendering/Texture.h>

#define MAX_INPUT_BUFFER_LENGTH 128

namespace Epoch
{
	enum class ContentBrowserAction
	{
		None                = 0,
		Refresh             = BIT(0),
		ClearSelections     = BIT(1),
		Selected			= BIT(2),
		Deselected			= BIT(3),
		Hovered             = BIT(4),
		Renamed             = BIT(5),
		OpenDeleteDialogue  = BIT(6),
		SelectToHere        = BIT(7),
		Moved               = BIT(8),
		ShowInExplorer      = BIT(9),
		OpenExternal        = BIT(10),
		Reload              = BIT(11),
		Copy				= BIT(12),
		Duplicate			= BIT(13),
		StartRenaming		= BIT(14),
		Activated			= BIT(15)
	};

	struct CBItemActionResult
	{
		uint16_t field = 0;

		void Set(ContentBrowserAction aFlag, bool aValue)
		{
			if (aValue)
			{
				field |= (uint16_t)aFlag;
			}
			else
			{
				field &= ~(uint16_t)aFlag;
			}
		}

		bool IsSet(ContentBrowserAction aFlag) const { return (uint16_t)aFlag & field; }
	};

	class ContentBrowserItem
	{
	public:
		enum class ItemType : uint16_t
		{
			Directory, Asset
		};
	public:
		ContentBrowserItem(ItemType aType, AssetHandle aId, const std::string& aName, const std::shared_ptr<Texture2D>& aIcon);
		virtual ~ContentBrowserItem() = default;

		void OnRenderBegin();
		CBItemActionResult OnRender();
		void OnRenderEnd();

		virtual void Delete() {}
		virtual bool Move(const std::filesystem::path& aDestination) { return false; }

		AssetHandle GetID() const { return myID; }
		ItemType GetType() const { return myType; }
		const std::string& GetName() const { return myFileName; }

		const std::shared_ptr<Texture2D>& GetIcon() const { return myIcon; }

		void StartRenaming();
		void StopRenaming();
		bool IsRenaming() const { return myIsRenaming; }

		void Rename(const std::string& aNewName);
		void SetDisplayNameFromFileName();

	private:
		virtual void OnRenamed(const std::string& aNewName) { myFileName = aNewName; }
		virtual void RenderCustomContextItems() {}
		virtual void UpdateDrop(CBItemActionResult& aActionResult) {}

		void OnContextMenuOpen(CBItemActionResult& aActionResult);

	protected:
		ItemType myType;
		AssetHandle myID;
		std::string myDisplayName;
		std::string myFileName;
		std::shared_ptr<Texture2D> myIcon;

		bool myIsRenaming = false;
		bool myIsDragging = false;
		bool myJustSelected = false;

		friend class ContentBrowserPanel;
	};

	struct DirectoryInfo
	{
		AssetHandle handle;
		std::shared_ptr<DirectoryInfo> parent;

		std::filesystem::path filePath;

		std::vector<AssetHandle> assets;

		std::map<AssetHandle, std::shared_ptr<DirectoryInfo>> subDirectories;
	};

	class ContentBrowserDirectory : public ContentBrowserItem
	{
	public:
		ContentBrowserDirectory(const std::shared_ptr<DirectoryInfo>& aDirectoryInfo);
		~ContentBrowserDirectory() = default;

		std::shared_ptr<DirectoryInfo>& GetDirectoryInfo() { return myDirectoryInfo; }

		void Delete() override;
		bool Move(const std::filesystem::path& aDestination) override;

	private:
		void OnRenamed(const std::string& aNewName) override;
		void UpdateDrop(CBItemActionResult& aActionResult) override;

	private:
		std::shared_ptr<DirectoryInfo> myDirectoryInfo;
	};

	class ContentBrowserAsset : public ContentBrowserItem
	{
	public:
		ContentBrowserAsset(const AssetMetadata& aAssetInfo, const std::shared_ptr<Texture2D>& aIcon);
		virtual ~ContentBrowserAsset() = default;

		const AssetMetadata& GetAssetInfo() const { return myAssetInfo; }

		virtual void Delete() override;
		virtual bool Move(const std::filesystem::path& aSestination) override;

	private:
		virtual void OnRenamed(const std::string& aNewName) override;

	private:
		AssetMetadata myAssetInfo;
	};
}
