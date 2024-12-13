#pragma once
#include <memory>
#include <filesystem>
#include <CommonUtilities/Math/Vector/Vector2.hpp>
#include <Epoch/Core/Layer.h>
#include <Epoch/Scene/Scene.h>
#include <Epoch/Scene/SceneRenderer.h>
#include <Epoch/Editor/EditorCamera.h>
#include <Epoch/Rendering/Texture.h>
#include <Epoch/Rendering/DebugRenderer.h>
#include "Aeon/PanelManager.h"

namespace ImGuizmo
{
	enum OPERATION;
}

namespace Epoch
{
	class KeyPressedEvent;
	class MouseButtonPressedEvent;
	class EditorFileDroppedEvent;

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
		void BeginViewportDockspace();
		void EndViewportDockspace();

		void RecursivePanelMenuItem(const std::vector<std::string>& aNameParts, uint32_t aDepth, bool& aIsOpen);

		std::pair<float, float> GetMouseViewportCord() const;
		std::pair<float, float> GetMouseViewportSpace() const;
		bool MouseInViewport();

		void HandleAssetDrop();

		void ShowCreateProjectPopup();

		//Renders component Gizmos and lines
		void OnRenderOverlay();
		void OnRenderColliders(Entity aEntity);
		//Calls OnDrawGizmos & OnDrawGizmosSelected
		void OnRenderGizmos();

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

		void ToolbarPanel();
		void ViewportPanel();

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

		void OnSetToEditorCameraTransform(Entity aEntity);
		void OnResetBoneTransforms(Entity aEntity);
		void OnCurrentSceneRenamed(const AssetMetadata& aMetadata);

	private:
		std::unique_ptr<PanelManager> myPanelManager;
		
		std::vector<std::function<void()>> myPostSceneUpdateQueue;

		bool myViewportFocused = false;
		bool myViewportHovered = false;
		bool myStartedCameraClickInViewport = false;
		struct Bounds
		{
			CU::Vector2f min;
			CU::Vector2f max;
		} myViewportBounds;

		EditorCamera myEditorCamera;
		bool myAllowEditorCameraMovement = false;

		std::shared_ptr<SceneRenderer> mySceneRenderer;
		std::shared_ptr<DebugRenderer> myDebugRenderer;

		std::shared_ptr<Scene> myActiveScene;
		std::shared_ptr<Scene> myEditorScene;
		std::filesystem::path myEditorScenePath;
		float myTimeSinceLastSave = 0.0f;

		enum class SceneState { Edit, Simulate, Play };
		SceneState mySceneState = SceneState::Edit;

		enum class GizmoOperation { None, Translate, Rotate, Scale };
		GizmoOperation myGizmoOperation = GizmoOperation::None;

		bool myDisplayCurrentColorGradingLUT = false;
		bool myShowGizmos = true;
		float myGizmoScale = 0.5f;

		struct DragSelectionBox
		{
			CU::Vector2f start;
			CU::Vector2f end;
			bool dragging = false;
		} myDragSelectionBox;
	};
}
