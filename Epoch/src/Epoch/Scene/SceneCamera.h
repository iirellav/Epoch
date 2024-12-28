#pragma once
#include "Epoch/Rendering/Camera.h"

namespace Epoch
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective, Orthographic };

		SceneCamera();
		~SceneCamera() = default;

		void SetViewportSize(unsigned aWidth, unsigned aHeight);
		float GetAspectRatio() const { return myAspectRatio; }

		void SetPerspective(float aFOV, float aNearClip, float aFarClip);
		void SetOrthographic(float aSize, float aNearClip, float aFarClip);

		ProjectionType GetProjectionType() const { return myProjectionType; }
		void SetProjectionType(ProjectionType type) { myProjectionType = type; }

		float GetPerspectiveFOV() const { return myPerspectiveFOV; }
		void SetPerspectiveFOV(float aFov) { myPerspectiveFOV = aFov; }
		float GetPerspectiveNearPlane() const { return myPerspectiveNear; }
		void SetPerspectiveNearPlane(float aNearClip) { myPerspectiveNear = aNearClip; }
		float GetPerspectiveFarPlane() const { return myPerspectiveFar; }
		void SetPerspectiveFarPlane(float aFarClip) { myPerspectiveFar = aFarClip; }

		float GetOrthographicSize() const { return myOrthographicSize; }
		void SetOrthographicSize(float aSize) { myOrthographicSize = aSize; }
		float GetOrthographicNearPlane() const { return myOrthographicNear; }
		void SetOrthographicNearPlane(float aNearClip) { myOrthographicNear = aNearClip; }
		float GetOrthographicFarPlane() const { return myOrthographicFar; }
		void SetOrthographicFarPlane(float aFarClip) { myOrthographicFar = aFarClip; }

	private:
		void RecalculateProjection();

	private:
		ProjectionType myProjectionType = ProjectionType::Perspective;

		float myPerspectiveFOV = 90.0f * CU::Math::ToRad;
		float myPerspectiveNear = 1.0f;
		float myPerspectiveFar = 25000.0f;;

		float myOrthographicSize = 1000.0f;
		float myOrthographicNear = 0.0f;
		float myOrthographicFar = 10000.0f;

		float myAspectRatio = 0.0f;
		unsigned myViewportWidth;
		unsigned myViewportHeight;
	};
}