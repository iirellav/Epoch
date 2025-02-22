#pragma once
#include <memory>
#include <filesystem>
#include <CommonUtilities/Math/Vector/Vector2.hpp>
#include <Epoch/Core/Layer.h>
#include <Epoch/Scene/Scene.h>
#include <Epoch/Editor/EditorCamera.h>
#include "PanelManager.h"

namespace ImGuizmo
{
	enum OPERATION;
}

namespace Epoch
{
	class KeyPressedEvent;
	class MouseButtonPressedEvent;
	class EditorFileDroppedEvent;

	class DebugRenderer;

	class ViewportPanel;
	class StatisticsPanel;

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate() override;
		void OnImGuiRender() override;

		void OnEvent(Event& aEvent) override;

	private:
		void BeginDockspace();
		void EndDockspace();

		void OnRenderMenuBar();

		void RecursivePanelMenuItem(const std::vector<std::string>& aNameParts, uint32_t aDepth, bool& aIsOpen);

		Entity GetHoveredEntity();

		void HandleAssetDrop();

		void ShowCreateProjectPopup();

		//Renders component Gizmos and lines
		void OnRenderOverlay();
		void OnRenderColliders(Entity aEntity);

		ImGuizmo::OPERATION GetGizmoOperation();
		void DrawGizmos();
		float GetGizmoSnapValue();

		void CreateProject(const std::filesystem::path& aPath);
		void SaveProject();
		void CloseProject();

		bool OpenProject();
		void OpenProject(const std::filesystem::path& aPath);

		void UpdateWindowTitle(const std::string& aSceneName);

		void OpenScene();
		bool OpenScene(const std::filesystem::path& aPath, const bool aCheckAutoSave = true);
		void NewScene(const std::string& aName = "New Scene");
		void SaveScene();
		void SaveSceneAs();
		void SaveSceneAuto();
		void SerializeScene(std::shared_ptr<Scene> aScene, const std::filesystem::path& aPath);
		void QueueSceneTransition(AssetHandle aScene);

		void EditorOptionsPanel();
		void DisplayColorGradingLUT();
		void ToolbarPanel();

		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();
		void OnSceneTransition(AssetHandle aScene);

		void OnEntityCreated(Entity aEntity);
		void OnEntityDeleted(Entity aEntity);

		bool OnKeyPressedEvent(KeyPressedEvent& aEvent);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& aEvent);

		bool OnViewportClickSelection();
		void UpdateViewportBoxSelection();

		void OnSnapToEditorCamera(Entity aEntity);
		void OnResetBoneTransforms(Entity aEntity);
		void OnCurrentSceneRenamed(const AssetMetadata& aMetadata);

	private:
		std::unique_ptr<PanelManager> myPanelManager;
		std::shared_ptr<ViewportPanel> mySceneViewport;
		std::shared_ptr<ViewportPanel> myGameViewport;
		std::shared_ptr<StatisticsPanel> myStatisticsPanel;

		std::vector<std::function<void()>> myPostSceneUpdateQueue;

		EditorCamera myEditorCamera;

		std::shared_ptr<DebugRenderer> myDebugRenderer;

		std::shared_ptr<Scene> myActiveScene;
		std::shared_ptr<Scene> myEditorScene;
		std::filesystem::path myEditorScenePath;
		float myTimeSinceLastSave = 0.0f;

		enum class SceneState { Edit, Simulate, Play };
		SceneState mySceneState = SceneState::Edit;

		enum class GizmoOperation { None, Translate, Rotate, Scale };
		GizmoOperation myGizmoOperation = GizmoOperation::None;

		bool myShowGizmos = true;
		float myGizmoScale = 0.5f;

		bool myDisplayUIInSceneView = false;
		bool myDisplayUIDebugRectsInSceneView = false;
		bool myPostProcessingInSceneView = true;

		bool myDisplayCurrentColorGradingLUT = false;
		bool myDebugRendererOnTop = false;

		enum class DebugLinesDrawMode { Off, All, Selected };
		DebugLinesDrawMode myShowBoundingBoxesMode = DebugLinesDrawMode::Off;
		DebugLinesDrawMode myShowCollidersMode = DebugLinesDrawMode::Off;

		bool myCullWithSceneCamera = false;

		struct DragSelectionBox
		{
			CU::Vector2f start;
			CU::Vector2f end;
			bool dragging = false;
		} myDragSelectionBox;
	};
}
