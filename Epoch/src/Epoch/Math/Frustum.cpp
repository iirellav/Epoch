#include "epch.h"
#include "Frustum.h"

namespace Epoch
{
    std::array<CU::Vector4f, 8> Frustum::GetCorners(const CU::Matrix4x4f& aInvViewProj)
    {
        std::array<CU::Vector4f, 8> corners =
        {
            CU::Vector4f(-1.0f, -1.0f, 0.0f, 1.0f),
            CU::Vector4f(-1.0f,  1.0f, 0.0f, 1.0f),
            CU::Vector4f( 1.0f,  1.0f, 0.0f, 1.0f),
            CU::Vector4f( 1.0f, -1.0f, 0.0f, 1.0f),

            CU::Vector4f(-1.0f, -1.0f, 1.0f, 1.0f),
            CU::Vector4f(-1.0f,  1.0f, 1.0f, 1.0f),
            CU::Vector4f( 1.0f,  1.0f, 1.0f, 1.0f),
            CU::Vector4f( 1.0f, -1.0f, 1.0f, 1.0f)
        };

        for (CU::Vector4f& corner : corners)
        {
            corner = corner * aInvViewProj;

            const float mag = 1.0f / corner.w;
            corner.x /= mag;
            corner.y /= mag;
            corner.z /= mag;
            corner.w /= mag;
        }

        return corners;
    }
}
