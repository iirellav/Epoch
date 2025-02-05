#pragma once
#include <memory>
#include <vector>
#include <functional>
#include <Epoch/Core/Layer.h>

namespace Epoch
{
	class Scene;
	class SceneRenderer;
	class RenderPipeline;
	class AssetPack;

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer(std::string_view aProjectPath);
		~RuntimeLayer() = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate() override;
		
	private:
		void OpenProject();
		void LoadScene(uint64_t aSceneHandle);

		void OnScenePlay();
		void OnSceneStop();
		
		void QueueSceneTransition(uint64_t aScene);

	private:
		std::shared_ptr<RenderPipeline> myCompositePipeline;
		
		std::string myProjectPath;
		std::shared_ptr<AssetPack> myAssetPack;

		std::shared_ptr<Scene> myRuntimeScene;
		std::shared_ptr<SceneRenderer> mySceneRenderer;

		std::vector<std::function<void()>> myPostSceneUpdateQueue;

		uint32_t myViewportWidth = 0;
		uint32_t myViewportHeight = 0;
	};
}
