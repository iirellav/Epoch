#pragma once
#include <memory>
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	class SceneRenderer;
	class DebugRenderer;

	class StatisticsPanel : public EditorPanel
	{
	public:
		StatisticsPanel(const std::string& aName);
		~StatisticsPanel() override = default;

		void OnImGuiRender(bool& aIsOpen) override;
		void OnSceneChanged(const std::shared_ptr<Scene>& aScene) override { mySceneContext = aScene; }
		
		void SetSceneRenderer(std::shared_ptr<SceneRenderer> aRenderer) { mySceneRendererReference = aRenderer; }
		void SetDebugRenderer(std::shared_ptr<DebugRenderer> aRenderer) { myDebugRendererReference = aRenderer; }

	private:
		std::shared_ptr<Scene> mySceneContext;
		std::weak_ptr<SceneRenderer> mySceneRendererReference;
		std::weak_ptr<DebugRenderer> myDebugRendererReference;
	};
}
