#include "epch.h"
#include "Project.h"
#include "Epoch/Debug/Log.h"

namespace Epoch
{
	void Project::SetActive(std::shared_ptr<Project> aProject)
	{
		EPOCH_PROFILE_FUNC();

		if (staticActiveProject)
		{
			staticAssetManager = nullptr;
		}

		staticActiveProject = aProject;
		if (staticActiveProject)
		{
			staticAssetManager = std::make_shared<EditorAssetManager>();
			staticAssetManager->LoadBuiltInAssets();
		}
	}

	void Project::SetActiveRuntime(std::shared_ptr<Project> aProject, std::shared_ptr<AssetPack> aAssetPack)
	{
		EPOCH_PROFILE_FUNC();

		if (staticActiveProject)
		{
			staticAssetManager = nullptr;
		}

		staticActiveProject = aProject;
		if (staticActiveProject)
		{
			staticAssetManager = std::make_shared<RuntimeAssetManager>();
			Project::GetRuntimeAssetManager()->SetAssetPack(aAssetPack);
			staticAssetManager->LoadBuiltInAssets();
		}
	}
}
