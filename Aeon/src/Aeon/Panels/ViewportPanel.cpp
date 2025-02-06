#include "ViewportPanel.h"
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Editor/PanelIDs.h>

namespace Epoch
{
	ViewportPanel::ViewportPanel(const std::string& aName) : EditorPanel(aName)
	{
		mySceneRenderer = std::make_shared<SceneRenderer>();
	}

	void ViewportPanel::OnImGuiRender(bool& aIsOpen)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		bool open = ImGui::Begin(myName.c_str());

		if (myGrabFocus)
		{
			myGrabFocus = false;
			ImGui::SetKeyboardFocusHere(-1);
			ImGui::FocusWindow(ImGui::GetCurrentWindow());

			open = true;
		}

		myIsVisible = open;
		if (!open)
		{
			ImGui::End();
			ImGui::PopStyleVar();
			return;
		}

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		myBounds.min = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		myBounds.max = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		myIsHovered = ImGui::IsWindowHovered();
		myIsFocused = ImGui::IsWindowFocused();
		myAllowEditorCameraMovement = (ImGui::IsMouseHoveringRect(myBounds.min, myBounds.max) && myIsFocused) || myStartedCameraClickInPanel;

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		mySize = { (uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y };
		
		mySceneRenderer->SetViewportSize(mySize.x, mySize.y);

		ImGui::Image((ImTextureID)mySceneRenderer->GetFinalPassTexture()->GetView(), viewportPanelSize);

		for (const auto& function : myAdditionalFunctions)
		{
			function();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ViewportPanel::OnSceneChanged(const std::shared_ptr<Scene>& aScene)
	{
		mySceneRenderer->SetScene(aScene);
	}
}
