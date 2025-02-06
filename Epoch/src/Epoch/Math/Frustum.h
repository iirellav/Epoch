#pragma once
#include <array>
#include <CommonUtilities/Math/Vector/Vector.h>
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>

namespace Epoch
{
	struct Frustum
	{
		struct Plane
		{
			CU::Vector3f normal;
			float distance = 0.0f;

			Plane() = default;

			Plane(const CU::Vector3f& aPoint0, const CU::Vector3f& aPoint1, const CU::Vector3f& aPoint2)
			{
				CU::Vector3f v1 = aPoint1 - aPoint0;
				CU::Vector3f v2 = aPoint2 - aPoint0;
				normal = v1.Cross(v2);
				normal.Normalize();

				// Compute D (distance)
				distance = -normal.Dot(aPoint0);
			}

			Plane(float x, float y, float z, float w) : normal({ x, y, z }), distance(w) {}
		};

#pragma warning( disable : 4201 )
		union
		{
			struct
			{
				Plane nearPlane;
				Plane rightPlane;
				Plane leftPlane;
				Plane topPlane;
				Plane bottomPlane;
				Plane farPlane;
			};
			
			std::array<Plane, 6> planes;
		};
#pragma warning( default : 4201 )

		Frustum()
		{
			planes = { Plane() };
		}

		static std::array<CU::Vector4f, 8> GetCorners(const CU::Matrix4x4f& aView, const CU::Matrix4x4f& aProj);
	};
}
