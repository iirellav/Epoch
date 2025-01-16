#include "epch.h"
#include <unordered_set>
#include "AssetPack.h"
#include "AssetPackSerializer.h"
#include "Epoch/Core/Platform.h"
#include "Epoch/Project/Project.h"
#include "Epoch/Scene/SceneSerializer.h"

namespace Epoch
{
    static void GetAllReferencedScenes(AssetHandle aCurrentScene, std::unordered_set<AssetHandle>& outReferencedScenes)
    {
        std::unordered_set<AssetHandle> referencedScenes;
        //Load current scene and store all referenced scenes in a set
        {
            const AssetRegistry& registry = Project::GetEditorAssetManager()->GetAssetRegistry();

            const auto& metadata = registry.Get(aCurrentScene);
            std::shared_ptr<Scene> scene = std::make_shared<Scene>(aCurrentScene);
            SceneSerializer serializer(scene);
            LOG_DEBUG("Deserializing Scene: {}", metadata.filePath);
            if (serializer.Deserialize(Project::GetAssetDirectory() / metadata.filePath))
            {
                //Only add the scene handle to 'referencedScenes' if it's not already in 'outReferencedScenes'
            }
            else
            {
                CONSOLE_LOG_ERROR("Failed to deserialize scene: {} ({})", metadata.filePath, aCurrentScene);
            }
        }

        outReferencedScenes.insert(referencedScenes.begin(), referencedScenes.end());

        //Call this function for each scene in the set
        for (AssetHandle sceneHandle : referencedScenes)
        {
            GetAllReferencedScenes(sceneHandle, outReferencedScenes);
        }
    }

    bool AssetPack::CreateFromActiveProject(std::atomic<float>& aProgress)
    {
        AssetPackFile assetPackFile;
        assetPackFile.header.buildVersion = Platform::GetCurrentDateTimeU64();

        aProgress = 0.0f;

        AssetHandle startSceneHandle = Project::GetActive()->GetConfig().startScene;
        const AssetRegistry& registry = Project::GetEditorAssetManager()->GetAssetRegistry();

        if (startSceneHandle == 0 || !registry.Contains(startSceneHandle))
        {
            myLastBuildError == BuildError::NoStartScene;
            return false;
        }

        std::unordered_set<AssetHandle> referencedScenes;
        referencedScenes.insert(startSceneHandle);
        GetAllReferencedScenes(startSceneHandle, referencedScenes);
        aProgress += 0.1f;

        float progressIncrement = 0.4f / (float)referencedScenes.size();

        std::unordered_set<AssetHandle> fullAssetList;
        for (AssetHandle sceneHandle : referencedScenes)
        {
            const auto& metadata = registry.Get(sceneHandle);
            std::shared_ptr<Scene> scene = std::make_shared<Scene>(sceneHandle);
            SceneSerializer serializer(scene);
            LOG_DEBUG("Deserializing Scene: {}", metadata.filePath);
            if (serializer.Deserialize(Project::GetAssetDirectory() / metadata.filePath))
            {
            }
            else
            {
                CONSOLE_LOG_ERROR("Failed to deserialize scene: {} ({})", metadata.filePath, sceneHandle);
            }

            aProgress += progressIncrement;
        }

        CONSOLE_LOG_INFO("Project contains {} used assets", fullAssetList.size());

        Buffer appBinary;
        if (std::filesystem::exists(Project::GetScriptModuleFilePath()))
        {
            appBinary = FileSystem::ReadBytes(Project::GetScriptModuleFilePath());
        }

        AssetPackSerializer assetPackSerializer;
        assetPackSerializer.Serialize(Project::GetAssetDirectory() / "AssetPack.eap", assetPackFile, appBinary, aProgress);

        aProgress = 1.0f;
        return true;
    }

    std::shared_ptr<AssetPack> AssetPack::LoadActiveProject()
    {
        return std::shared_ptr<AssetPack>();
    }

    const char* AssetPack::GetLastErrorMessage()
    {
        return nullptr;
    }
}
