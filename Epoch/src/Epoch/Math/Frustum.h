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
			//CU::Vector3f normal;
			//float distance = 0.0f;

			CU::Vector3f point;
			CU::Vector3f normal;

			Plane() = default;
			Plane(const CU::Vector3f& aPoint, const CU::Vector3f& aNormal) : normal(aNormal.GetNormalized()), point(aPoint) {}
			//Plane(const CU::Vector3f& aPoint, const CU::Vector3f& aNormal) : normal(aNormal.GetNormalized()), distance(aNormal.Dot(aPoint)) {}
			//Plane(const CU::Vector4f& aVector4) : normal(aVector4), distance(aVector4.w) {}
			//Plane(float x, float y, float z, float w) : normal({ x, y, z }), distance(w) {}
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

		//NOTE: Not working
		static std::array<CU::Vector4f, 8> GetCorners(const CU::Matrix4x4f& aView, const CU::Matrix4x4f& aProj);
	};
}
