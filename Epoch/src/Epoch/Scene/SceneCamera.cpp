#include "epch.h"
#include "SceneCamera.h"

namespace Epoch
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(unsigned aWidth, unsigned aHeight)
	{
		EPOCH_ASSERT(aWidth > 0 && aHeight > 0, "Cameras viewport size can not be 0!");

		myViewportWidth = aWidth; myViewportHeight = aHeight;
		myAspectRatio = (float)aWidth / (float)aHeight;

		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float aFOV, float aNearClip, float aFarClip)
	{
		myProjectionType = ProjectionType::Perspective;
		myPerspectiveFOV = aFOV;
		myPerspectiveNear = aNearClip;
		myPerspectiveFar = aFarClip;
	}

	void SceneCamera::SetOrthographic(float aSize, float aNearClip, float aFarClip)
	{
		myProjectionType = ProjectionType::Orthographic;
		myOrthographicSize = aSize;
		myOrthographicNear = aNearClip;
		myOrthographicFar = aFarClip;
	}

	void SceneCamera::RecalculateProjection()
	{
		EPOCH_PROFILE_FUNC();

		if (myProjectionType == ProjectionType::Perspective)
		{
			myProjectionMatrix = CU::Matrix4x4f::CreatePerspectiveProjection(myPerspectiveFOV, myPerspectiveNear, myPerspectiveFar, myAspectRatio);
		}
		else
		{
			float orthoLeft = -myOrthographicSize * myAspectRatio * 0.5f;
			float orthoRight = myOrthographicSize * myAspectRatio * 0.5f;
			float orthoBottom = -myOrthographicSize * 0.5f;
			float orthoTop = myOrthographicSize * 0.5f;

			myProjectionMatrix = CU::Matrix4x4f::CreateOrthographicProjection(orthoLeft, orthoRight, orthoBottom, orthoTop, myOrthographicNear, myOrthographicFar);
		}
	}
}
