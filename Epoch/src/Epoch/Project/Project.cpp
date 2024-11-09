#include "epch.h"
#include "Project.h"

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
			std::shared_ptr<EditorAssetManager> assetManager = std::make_shared<EditorAssetManager>();
			staticAssetManager = assetManager;
			assetManager->LoadBuiltInAssets();
		}
	}

	void Project::SetActiveRuntime(std::shared_ptr<Project> aProject)
	{
		EPOCH_PROFILE_FUNC();

		if (staticActiveProject)
		{
			staticAssetManager = nullptr;
		}

		staticActiveProject = aProject;
		if (staticActiveProject)
		{
			std::shared_ptr<EditorAssetManager> assetManager = std::make_shared<EditorAssetManager>();
			staticAssetManager = assetManager;
			assetManager->LoadBuiltInAssets();
		}
	}
}
