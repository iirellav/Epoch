#include "epch.h"
#include "EditorCamera.h"
#include <CommonUtilities/Input/InputHandler.h>
#include <CommonUtilities/Timer.h>
#include "Epoch/ImGui/UICore.h"
#include "Epoch/Core/Input.h"

namespace Epoch
{
	EditorCamera::EditorCamera(float aFov, float aAspectRatio, float aNearPlane, float aFarPlane) : Camera(aFov, aAspectRatio, aNearPlane, aFarPlane)
	{
		myFov = aFov;
		myAspectRatio = aAspectRatio;
		myNearPlane = aNearPlane;
		myFarPlane = aFarPlane;
	}

	static void DisableMouse()
	{
		UI::SetInputEnabled(false);
		Input::SetCursorMode(CursorMode::Locked);
	}

	static void EnableMouse()
	{
		UI::SetInputEnabled(true);
		Input::SetCursorMode(CursorMode::Normal);
	}

	void EditorCamera::OnUpdate()
	{
		if (!myIsActive)
		{
			//if (!UI::IsInputEnabled())
			//{
			//	UI::SetInputEnabled(true);
			//}

			return;
		}

		const CU::Vector2f delta = -Input::GetMouseDelta() * 0.002f;

		if (Input::IsMouseButtonDown(MouseButton::Right))
		{
			//DisableMouse();

			float scrollDelta = Input::GetMouseScroll().y;
			mySpeedMultiplier += scrollDelta * 0.3f;
			mySpeedMultiplier = CU::Math::Clamp(mySpeedMultiplier, 0.1f, 4.0f);

			MouseRotate(delta);
		}
		else
		{
			//EnableMouse();
		}
	}

	void EditorCamera::Focus(const CU::Vector3f& aFocusPoint)
	{
		myTransform.SetTranslation(aFocusPoint - myTransform.GetForward() * 500.0f);
	}

	void EditorCamera::SetViewportSize(unsigned aWidth, unsigned aHeight)
	{
		//EPOCH_ASSERT(aWidth > 0 && aHeight > 0, "Cameras viewport size can not be 0!");
		if (aWidth == 0 || aHeight == 0) return;
		if (myViewportWidth == aWidth && myViewportHeight == aHeight) return;
		myViewportWidth = aWidth; myViewportHeight = aHeight;
		myAspectRatio = (float)myViewportWidth / (float)myViewportHeight; //TODO: Figure out how this even works, shouldn't it be height/width?
		UpdateProjection();
	}

	void EditorCamera::MouseRotate(const CU::Vector2f& aDelta)
	{
		myTransform.Rotate(CU::Vector3f(aDelta.y, aDelta.x, 0)/* * 2.5f*/);

		CU::Vector3f moveDelta;
		if (ImGui::IsKeyDown(ImGuiKey_W)) moveDelta.z += 1.0f;
		if (ImGui::IsKeyDown(ImGuiKey_S)) moveDelta.z -= 1.0f;
		if (ImGui::IsKeyDown(ImGuiKey_D)) moveDelta.x += 1.0f;
		if (ImGui::IsKeyDown(ImGuiKey_A)) moveDelta.x -= 1.0f;
		if (ImGui::IsKeyDown(ImGuiKey_E)) moveDelta.y += 1.0f;
		if (ImGui::IsKeyDown(ImGuiKey_Q)) moveDelta.y -= 1.0f;

		CU::Vector3f movement = myTransform.GetRight() * moveDelta.x + myTransform.GetForward() * moveDelta.z + myTransform.GetUp() * moveDelta.y;

		const float moveSpeed = (ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 1200.0f : 400.0f) * mySpeedMultiplier;
		//const float moveSpeed = (ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 2000.0f : 800.0f) * mySpeedMultiplier;
		myTransform.Translate(movement.GetNormalized() * moveSpeed * CU::Timer::GetDeltaTime());
	}

	void EditorCamera::UpdateProjection()
	{
		myProjectionMatrix = CU::Matrix4x4f::CreatePerspectiveProjection(myFov, myNearPlane, myFarPlane, myAspectRatio);
	}
}
