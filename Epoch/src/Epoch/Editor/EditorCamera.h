#pragma once
#include <CommonUtilities/Math/Transform.h>
#include "Epoch/Debug/Log.h"
#include "Epoch/Rendering/Camera.h"

namespace Epoch
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float aFov, float aAspectRatio = 1.778f, float aNearPlane = 5.0f, float aFarPlane = 100000.0f);
		~EditorCamera() override = default;

		void OnUpdate();
		void Focus(const CU::Vector3f& aFocusPoint);

		void SetActive(bool aState) { myIsActive = aState; }

		CU::Matrix4x4f GetViewMatrix() { return myTransform.GetMatrix().GetFastInverse(); }

		CU::Transform& GetTransform() { return myTransform; }
		const CU::Transform& GetTransform() const { return myTransform; }
		const CU::Matrix4x4f& GetTransformMatrix() { return myTransform.GetMatrix(); }

		float GetFOV() const { return myFov; }
		float GetAspectRatio() const { return myAspectRatio; }
		float GetNearPlane() const { return myNearPlane; }
		float GetFarPlane() const { return myFarPlane; }

		void SetViewportSize(unsigned aWidth, unsigned aHeight);

	private:
		void MouseRotate(const CU::Vector2f& aDelta);

		void UpdateProjection();

	protected:
		bool myIsActive = false;

		float myFov = 90 * CU::Math::ToRad;
		float myAspectRatio = 1.778f;
		float myNearPlane = 5.0f;
		float myFarPlane = 100000.0f;

		unsigned myViewportWidth = 1920;
		unsigned myViewportHeight = 1080;

		CU::Transform myTransform;
		float mySpeedMultiplier = 1.0f;
	};
}
