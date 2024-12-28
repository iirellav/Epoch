#include "epch.h"
#include "Frustum.h"

namespace Epoch
{
    std::array<CU::Vector4f, 8> Frustum::GetCorners(const CU::Matrix4x4f& aView, const CU::Matrix4x4f& aProj)
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

        std::array<CU::Vector4f, 8> outputCorners;
        for (size_t i = 0; i < 8; i++)
        {
            const CU::Vector4f& corner = corners[i];

            CU::Vector4f result = (aView * aProj).GetFastInverse() * corner;
            result = result / result.w;

            outputCorners[i] = result;
        }

        return outputCorners;
    }
}
