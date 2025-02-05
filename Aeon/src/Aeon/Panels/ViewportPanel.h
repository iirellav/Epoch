#pragma once
#include "Epoch/Editor/EditorPanel.h"
#include <Epoch/Scene/SceneRenderer.h>

namespace Epoch
{
	class ViewportPanel : public EditorPanel
	{
	public:
		ViewportPanel(const std::string& aName);
		~ViewportPanel() override = default;

		void OnImGuiRender(bool& aIsOpen) override;

		void OnSceneChanged(const std::shared_ptr<Scene>& aScene) override;

		void SetFocus() { myGrabFocus = true; }

		bool IsVisible() const { return myIsVisible; }

		bool IsFocused() const { return myIsFocused; }
		bool IsHovered() const { return myIsHovered; }
		
		void SetStartedCameraClickInPanel(bool aState) { myStartedCameraClickInPanel = aState; }
		bool GetStartedCameraClickInPanel() const { return myStartedCameraClickInPanel; }

		const CU::Vector2f& MaxBounds() const { return myBounds.max; }
		const CU::Vector2f& MinBounds() const { return myBounds.min; }

		const CU::Vector2ui& Size() const { return mySize; }

		std::shared_ptr<SceneRenderer> GetSceneRenderer() { return mySceneRenderer; }

		bool AllowEditorCameraMovement() const { return myAllowEditorCameraMovement; }
		
		void AddAdditionalFunction(const std::function<void()>& aFunc) { myAdditionalFunctions.push_back(aFunc); }

	private:
		std::shared_ptr<SceneRenderer> mySceneRenderer;

		bool myGrabFocus = false;

		bool myIsVisible = false;

		bool myIsFocused = false;
		bool myIsHovered = false;
		bool myStartedCameraClickInPanel = false;
		bool myAllowEditorCameraMovement = false;

		struct Bounds
		{
			CU::Vector2f min;
			CU::Vector2f max;
		} myBounds;

		CU::Vector2ui mySize;
		
		std::vector<std::function<void()>> myAdditionalFunctions;
	};
}
