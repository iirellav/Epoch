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
		
		ImGuiWindow* win1 = ImGui::GetCurrentWindow();
		ImGuiWindow* win2 = nullptr;

		myIsHovered = ImGui::IsWindowHovered();
		myIsFocused = ImGui::IsWindowFocused();
		myAllowEditorCameraMovement = (ImGui::IsMouseHoveringRect(myBounds.min, myBounds.max) && myIsFocused) || myStartedCameraClickInPanel;

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		mySize = { (uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y };
		
		mySceneRenderer->SetViewportSize(mySize.x, mySize.y);

		auto curPos = ImGui::GetCursorPos();

		ImGui::Image((ImTextureID)mySceneRenderer->GetFinalPassTexture()->GetView(), viewportPanelSize);

		for (const auto& function : myAdditionalFunctions)
		{
			function();
		}

		//if (myDisplayCurrentColorGradingLUT)
		//{
		//	AssetHandle lutHandle(0);
		//
		//	auto entities = myActiveScene->GetAllEntitiesWith<VolumeComponent>();
		//	for (auto entityID : entities)
		//	{
		//		Entity entity = Entity(entityID, myActiveScene.get());
		//		if (!entity.IsActive()) continue;
		//
		//		const auto& vc = entities.get<VolumeComponent>(entityID);
		//		if (!vc.isActive || !vc.colorGrading.enabled) continue;
		//
		//		lutHandle = vc.colorGrading.lut;
		//
		//		break;
		//	}
		//
		//	ImGui::SetCursorPos(curPos);
		//	UI::ShiftCursor(3, 2);
		//	if (mySceneRenderer->ColorGrading() && lutHandle != 0)
		//	{
		//		auto lut = AssetManager::GetAsset<Texture>(lutHandle);
		//		ImGui::Image((ImTextureID)lut->GetView(), { 256, 16 });
		//	}
		//	else
		//	{
		//		ImGui::Image((ImTextureID)Renderer::GetDefaultColorGradingLut()->GetView(), { 256, 16 });
		//	}
		//}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ViewportPanel::OnSceneChanged(const std::shared_ptr<Scene>& aScene)
	{
		mySceneRenderer->SetScene(aScene);
	}
}
