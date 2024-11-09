#pragma once
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>

namespace Epoch
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(float aFov, float aAspectRatio, float aNearPlane, float aFarPlane) : myProjectionMatrix(CU::Matrix4x4f::CreatePerspectiveProjection(aFov, aNearPlane, aFarPlane, aAspectRatio)) {}
		virtual ~Camera() = default;

		const CU::Matrix4x4f& GetProjectionMatrix() const { return myProjectionMatrix; }

	protected:
		CU::Matrix4x4f myProjectionMatrix = CU::Matrix4x4f::Zero;
	};
}
