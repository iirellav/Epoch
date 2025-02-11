#include "epch.h"
#include "EditorAssetManager.h"
#pragma warning(push, 0)
#include <yaml-cpp/yaml.h>
#pragma warning(pop)
#include "Epoch/Core/Application.h"
#include "Epoch/Project/Project.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Assets/AssetExtensions.h"
#include "Epoch/Rendering/MeshFactory.h"
#include "Epoch/Rendering/Material.h"

namespace Epoch
{
	static AssetMetadata staticNullMetadata;

	EditorAssetManager::EditorAssetManager()
	{
		EPOCH_PROFILE_FUNC();

		AssetImporter::Init();

		DeserializeAssetRegistry();
		ReloadAssets();
	}

	EditorAssetManager::~EditorAssetManager()
	{
		SerializeAssetRegistry();

		//for (auto [handle, asset] : myLoadedAssets)
		//{
		//	EPOCH_ASSERT(asset, "Loaded asset were nullptr!");
		//	EPOCH_ASSERT(asset.use_count() <= 2, "Loaded asset will not be destroyed - something is still holding refs!\n[{}] {}", handle, GetRelativePath(GetFileSystemPath(handle)));
		//}

		//for (auto [handle, asset] : myMemoryAssets)
		//{
		//	EPOCH_ASSERT(asset, "Memory asset were nullptr!");
		//	EPOCH_ASSERT(asset.use_count() <= 2, "Memory asset will not be destroyed - something is still holding refs!\n[{}] {}", handle, GetRelativePath(GetFileSystemPath(handle)));
		//}
	}

	void EditorAssetManager::LoadBuiltInAssets()
	{
		MeshFactory::CreateCube();
		MeshFactory::CreateSphere();
		MeshFactory::CreateQuad();
		MeshFactory::CreatePlane();

		AssetManager::CreateMemoryOnlyAssetWithName<Material>("Default-Material");
	}

	AssetType EditorAssetManager::GetAssetType(AssetHandle aHandle)
	{
		if (!IsAssetHandleValid(aHandle))
		{
			return AssetType::None;
		}

		const AssetMetadata& metadata = GetMetadata(aHandle);
		return metadata.type;
	}

	std::shared_ptr<Asset> EditorAssetManager::GetAsset(AssetHandle aHandle)
	{
		//EPOCH_PROFILE_FUNC();

		std::shared_ptr<Asset> asset = GetAssetIncludingInvalid(aHandle);
		return asset && asset->IsValid() ? asset : nullptr;
	}

	static std::shared_ptr<Asset> LoadAssetAsync(AssetMetadata aMetadata)
	{
		std::shared_ptr<Asset> asset;
		bool loaded = AssetImporter::TryLoadData(aMetadata, asset);
		if (loaded)
		{
			return asset;
		}

		return nullptr;
	}

	//TODO: This is kinda messy, maybe make a separate class to handle this
	std::shared_ptr<Asset> EditorAssetManager::GetAssetAsync(AssetHandle aHandle)
	{
		if (IsMemoryAsset(aHandle))
		{
			return myMemoryAssets[aHandle];
		}

		auto& metadata = GetMetadataInternal(aHandle);
		if (!metadata.IsValid())
		{
			return nullptr;
		}

		if (metadata.isDataLoaded)
		{
			EPOCH_ASSERT(myLoadedAssets.contains(aHandle), "\"Loaded\" asset not found in loaded assets!");
			std::shared_ptr<Asset> asset = myLoadedAssets[aHandle];
			return asset && asset->IsValid() ? asset : nullptr;
		}

		if (myLoadingAssets.find(aHandle) != myLoadingAssets.end())
		{
			auto& future = myLoadingAssets[aHandle];
			if (future._Is_ready())
			{
				std::shared_ptr<Asset> asset = future._Get_value();
				myLoadingAssets.erase(aHandle);

				if (asset)
				{
					myLoadedAssets.emplace(aHandle, asset);
					metadata.isDataLoaded = true;
				}
				return asset;
			}
			else
			{
				return nullptr;
			}
		}

		JobSystem& js = Application::Get().GetJobSystem();
		myLoadingAssets[aHandle] = js.AddAJob(LoadAssetAsync, metadata);
		return nullptr;
	}

	void EditorAssetManager::AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset)
	{
		AssetMetadata metadata;
		metadata.handle = aAsset->GetHandle();
		metadata.isDataLoaded = true;
		metadata.type = aAsset->GetAssetType();
		metadata.isMemoryAsset = true;
		myAssetRegistry[metadata.handle] = metadata;

		myMemoryAssets[aAsset->GetHandle()] = aAsset;
	}

	void EditorAssetManager::AddMemoryOnlyAsset(std::shared_ptr<Asset> aAsset, const std::string& aName)
	{
		AssetMetadata metadata;
		metadata.handle = aAsset->GetHandle();
		metadata.filePath = aName;
		metadata.isDataLoaded = true;
		metadata.type = aAsset->GetAssetType();
		metadata.isMemoryAsset = true;
		myAssetRegistry[metadata.handle] = metadata;

		myMemoryAssets[aAsset->GetHandle()] = aAsset;
	}

	bool EditorAssetManager::ReloadData(AssetHandle aAssetHandle)
	{
		bool result = false;
		auto& metadata = GetMetadataInternal(aAssetHandle);
		if (!metadata.IsValid())
		{
			LOG_ERROR_TAG("AssetManager", "Trying to reload invalid asset");
			return result;
		}

		std::shared_ptr<Asset> asset;
		metadata.isDataLoaded = AssetImporter::TryLoadData(metadata, asset);
		if (metadata.isDataLoaded)
		{
			myLoadedAssets[aAssetHandle] = asset;
		}
		result = metadata.isDataLoaded;

		return result;
	}

	void EditorAssetManager::RemoveAsset(AssetHandle aHandle)
	{
		if (myLoadedAssets.contains(aHandle))
		{
			myLoadedAssets.erase(aHandle);
		}

		if (myMemoryAssets.contains(aHandle))
		{
			myMemoryAssets.erase(aHandle);
		}

		if (myAssetRegistry.Contains(aHandle))
		{
			myAssetRegistry.Remove(aHandle);
		}
	}

	bool EditorAssetManager::IsMemoryAsset(AssetHandle aHandle)
	{
		return myMemoryAssets.find(aHandle) != myMemoryAssets.end();
	}

	bool EditorAssetManager::IsAssetHandleValid(AssetHandle aHandle)
	{
		return IsMemoryAsset(aHandle) || GetMetadata(aHandle).IsValid();
	}

	bool EditorAssetManager::IsAssetLoaded(AssetHandle aHandle)
	{
		return myLoadedAssets.contains(aHandle);
	}

	bool EditorAssetManager::IsAssetValid(AssetHandle aHandle)
	{
		std::shared_ptr<Asset> asset = GetAssetIncludingInvalid(aHandle);
		return asset && asset->IsValid();
	}

	bool EditorAssetManager::IsAssetMissing(AssetHandle aHandle)
	{
		if (IsMemoryAsset(aHandle))
		{
			return false;
		}

		AssetMetadata& metadata = GetMetadataInternal(aHandle);
		return !FileSystem::Exists(Project::GetAssetDirectory() / metadata.filePath);
	}

	std::unordered_set<AssetHandle> EditorAssetManager::GetAllAssetsWithType(AssetType aType)
	{
		std::unordered_set<AssetHandle> result;
		for (const auto& [handle, metadata] : myAssetRegistry)
		{
			if (metadata.type == aType)
			{
				result.insert(handle);
			}
		}
		return result;
	}

	const std::unordered_map<AssetHandle, std::shared_ptr<Asset>>& EditorAssetManager::GetLoadedAssets()
	{
		return myLoadedAssets;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle aHandle)
	{
		if (myAssetRegistry.Contains(aHandle))
		{
			return myAssetRegistry[aHandle];
		}

		return staticNullMetadata;
	}

	AssetMetadata& EditorAssetManager::GetMutableMetadata(AssetHandle aHandle)
	{
		if (myAssetRegistry.Contains(aHandle))
		{
			return myAssetRegistry[aHandle];
		}

		return staticNullMetadata;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(const std::filesystem::path& aFilepath)
	{
		const auto relativePath = GetRelativePath(aFilepath);

		for (auto& [handle, metadata] : myAssetRegistry)
		{
			if (metadata.filePath == relativePath)
			{
				return metadata;
			}
		}

		return staticNullMetadata;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(const std::shared_ptr<Asset>& aAsset)
	{
		return GetMetadata(aAsset->GetHandle());
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& aFilepath)
	{
		std::filesystem::path path = GetRelativePath(aFilepath);

		if (const auto& metadata = GetMetadata(path); metadata.IsValid())
		{
			return metadata.handle;
		}

		AssetType type = GetAssetTypeFromPath(path);
		if (type == AssetType::None)
		{
			return 0;
		}

		AssetMetadata metadata;
		metadata.handle = AssetHandle();
		metadata.filePath = path;
		metadata.type = type;

		myAssetRegistry[metadata.handle] = metadata;

		LOG_INFO_TAG("AssetManager", "Asset imported '{}'", metadata.filePath.string());

		return metadata.handle;
	}

	AssetHandle EditorAssetManager::GetAssetHandleFromFilePath(const std::filesystem::path& aFilepath)
	{
		return GetMetadata(aFilepath).handle;
	}

	AssetType EditorAssetManager::GetAssetTypeFromExtension(const std::string& aExtension)
	{
		std::string ext = CU::ToLower(aExtension);
		if (staticAssetExtensionMap.find(ext) == staticAssetExtensionMap.end())
		{
			return AssetType::None;
		}

		return staticAssetExtensionMap.at(ext);
	}

	AssetType EditorAssetManager::GetAssetTypeFromPath(const std::filesystem::path& aPath)
	{
		return GetAssetTypeFromExtension(aPath.extension().string());
	}

	std::filesystem::path EditorAssetManager::GetFileSystemPath(AssetHandle aHandle)
	{
		return GetFileSystemPath(GetMetadata(aHandle));
	}

	std::filesystem::path EditorAssetManager::GetFileSystemPath(const AssetMetadata& aMetadata)
	{
		return Project::GetAssetDirectory() / aMetadata.filePath;
	}

	std::filesystem::path EditorAssetManager::GetRelativePath(const std::filesystem::path& aFilepath)
	{
		std::filesystem::path relativePath = aFilepath.lexically_normal();
		std::string temp = aFilepath.string();
		if (temp.find(Project::GetAssetDirectory().string()) != std::string::npos)
		{
			relativePath = std::filesystem::relative(aFilepath, Project::GetAssetDirectory());
			if (relativePath.empty())
			{
				relativePath = aFilepath.lexically_normal();
			}
		}
		return relativePath;
	}

	bool EditorAssetManager::FileExists(AssetMetadata& aMetadata) const
	{
		return FileSystem::Exists(Project::GetAssetDirectory() / aMetadata.filePath);
	}

	void EditorAssetManager::OnAssetRenamed(AssetHandle aAssetHandle, const std::filesystem::path& aNewFilePath)
	{
		AssetMetadata& metadata = myAssetRegistry[aAssetHandle];
		if (!metadata.IsValid())
		{
			return;
		}

		metadata.filePath = GetRelativePath(aNewFilePath);
		SerializeAssetRegistry();
	}

	void EditorAssetManager::OnAssetDeleted(AssetHandle aAssetHandle)
	{
		AssetMetadata metadata = GetMetadata(aAssetHandle);
		if (!metadata.IsValid())
		{
			return;
		}

		myAssetRegistry.Remove(aAssetHandle);
		myLoadedAssets.erase(aAssetHandle);
		SerializeAssetRegistry();
	}

	void EditorAssetManager::SerializeAssetRegistry()
	{
		EPOCH_PROFILE_FUNC();

		LOG_INFO_TAG("AssetManager", "Serializing asset registry");
		uint32_t assetCount = 0;

		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Assets" << YAML::BeginSeq;
		for (auto& [filepath, metadata] : myAssetRegistry)
		{
			if (metadata.isMemoryAsset)
			{
				continue;
			}

			if (!FileSystem::Exists(GetFileSystemPath(metadata)))
			{
				LOG_WARNING_TAG("AssetManager", "Missing asset '{}' detected in registry", metadata.filePath.string());
				continue;
			}

			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << YAML::Value << metadata.handle;
			out << YAML::Key << "FilePath" << YAML::Value << metadata.filePath.string();
			out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.type);
			out << YAML::EndMap;

			assetCount++;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		const std::string& assetRegistryPath = Project::GetAssetRegistryPath().string();
		std::ofstream fout(assetRegistryPath);
		fout << out.c_str();

		LOG_INFO_TAG("AssetManager", "Serialized {} assets", assetCount);
	}

	void EditorAssetManager::DeserializeAssetRegistry()
	{
		EPOCH_PROFILE_FUNC();

		LOG_INFO_TAG("AssetManager", "Importing Asset Registry");

		const auto& assetRegistryPath = Project::GetAssetRegistryPath();
		if (!FileSystem::Exists(assetRegistryPath))
		{
			return;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetRegistryPath.string());
		}
		catch (YAML::ParserException e)
		{
			LOG_ERROR_TAG("AssetManager", "Failed to load asset registry '{}'\n     {}", assetRegistryPath.string(), e.what());
			return;
		}

		auto handles = data["Assets"];
		if (!handles)
		{
			LOG_WARNING_TAG("AssetManager", "Asset registry was empty '{}'", assetRegistryPath.string());
			return;
		}

		for (auto entry : handles)
		{
			std::string filepath = entry["FilePath"].as<std::string>();

			AssetMetadata metadata;
			metadata.handle = entry["Handle"].as<uint64_t>();
			metadata.filePath = filepath;
			metadata.type = AssetTypeFromString(entry["Type"].as<std::string>());

			if (metadata.type == AssetType::None)
			{
				continue;
			}

			if (metadata.type != GetAssetTypeFromPath(filepath))
			{
				LOG_WARNING_TAG("AssetManager", "Mismatch between stored asset types ({}) and extension type ({}) when reading asset registry! '{}'", AssetTypeToString(metadata.type), AssetTypeToString(GetAssetTypeFromPath(filepath)), filepath);
				metadata.type = GetAssetTypeFromPath(filepath);
			}

			if (!FileSystem::Exists(GetFileSystemPath(metadata)))
			{
				LOG_WARNING_TAG("AssetManager", "Missing asset '{}' detected in registry file", metadata.filePath.string());
				//LOG_WARNING("Missing asset '{}' detected in registry file, trying to locate...", metadata.filePath);
				continue;
			}

			if (AssetMetadata otherMetadata = GetMetadata(filepath); otherMetadata.IsValid())
			{
				LOG_WARNING_TAG("AssetManager", "Asset with filename '{}' already loaded from registry file", metadata.filePath.string());
			}

			if (metadata.handle == 0)
			{
				LOG_WARNING_TAG("AssetManager", "Asset handle for '{}' is 0, this is an invalid handle.", metadata.filePath.string());
				continue;
			}

			myAssetRegistry[metadata.handle] = metadata;
		}

		LOG_INFO_TAG("AssetManager", "Imported {} assets", myAssetRegistry.Count());
	}

	void EditorAssetManager::ProcessDirectory(const std::filesystem::path& aDirectoryPath)
	{
		for (const auto entry : std::filesystem::directory_iterator(aDirectoryPath))
		{
			if (entry.is_directory())
			{
				ProcessDirectory(entry.path());
			}
			else
			{
				ImportAsset(entry.path());
			}
		}
	}

	void EditorAssetManager::ReloadAssets()
	{
		{
			EPOCH_PROFILE_SCOPE("void Epoch::EditorAssetManager::ProcessDirectory(const std::filesystem::path &)");
			ProcessDirectory(Project::GetAssetDirectory().string());
		}
		SerializeAssetRegistry();
	}

	std::shared_ptr<Asset> EditorAssetManager::GetAssetIncludingInvalid(AssetHandle aAssetHandle)
	{
		std::shared_ptr<Asset> asset = nullptr;

		if (IsMemoryAsset(aAssetHandle))
		{
			asset = myMemoryAssets[aAssetHandle];
		}
		else
		{
			AssetMetadata& metadata = GetMetadataInternal(aAssetHandle);
			if (metadata.IsValid())
			{
				if (!metadata.isDataLoaded)
				{
					metadata.isDataLoaded = AssetImporter::TryLoadData(metadata, asset);
					if (metadata.isDataLoaded)
					{
						myLoadedAssets[aAssetHandle] = asset;
					}
				}
				else
				{
					asset = myLoadedAssets[aAssetHandle];
				}
			}
		}

		return asset;
	}

	AssetMetadata& EditorAssetManager::GetMetadataInternal(AssetHandle aHandle)
	{
		if (myAssetRegistry.Contains(aHandle))
		{
			return myAssetRegistry[aHandle];
		}

		return staticNullMetadata;
	}
}
