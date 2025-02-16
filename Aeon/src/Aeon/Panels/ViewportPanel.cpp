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

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		mySize = { (uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y };
		
		mySceneRenderer->SetViewportSize(mySize.x, mySize.y);

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

	std::pair<float, float> ViewportPanel::GetMouseViewportCord() const
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= myBounds.min.x;
		my -= myBounds.min.y;
		my = (float)mySize.y - my;

		return { mx, my };
	}

	std::pair<float, float> ViewportPanel::GetMouseViewportSpace() const
	{
		auto [mx, my] = GetMouseViewportCord();

		return { (mx / (float)mySize.x) * 2.0f - 1.0f, (my / (float)mySize.y) * 2.0f - 1.0f };
	}

	bool ViewportPanel::MouseInViewport()
	{
		if (!myIsVisible)
		{
			return false;
		}

		auto [mouseX, mouseY] = GetMouseViewportSpace();
		return (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f);
	}
}
