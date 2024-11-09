#include "epch.h"
#include <functional>
#include "MeshFactory.h"
#include "Epoch/Rendering/Mesh.h"
#include "Epoch/Assets/AssetManager.h"

namespace Epoch
{
	void MeshFactory::CreateCube()
	{
		const std::vector<Vertex> vertexList =
		{
			// Front
			{ 50, 50, 50, 0, 0, 1, -1, 0, 0, 0, 0 },
			{ -50, 50, 50, 0, 0, 1, -1, 0, 0, 1, 0 },
			{ 50, -50, 50, 0, 0, 1, -1, 0, 0, 0, 1 },
			{ -50, -50, 50, 0, 0, 1, -1, 0, 0, 1, 1 },

			// Left
			{ -50, 50, 50, -1, 0, 0, 0, 0, -1, 0, 0 },
			{ -50, 50, -50, -1, 0, 0, 0, 0, -1, 1, 0 },
			{ -50, -50, 50, -1, 0, 0, 0, 0, -1, 0, 1 },
			{ -50, -50, -50, -1, 0, 0, 0, 0, -1, 1, 1 },

			// Back
			{ -50, 50, -50, 0, 0, -1, 1, 0, 0, 0, 0 },
			{ 50, 50, -50, 0, 0, -1, 1, 0, 0, 1, 0 },
			{ -50, -50, -50, 0, 0, -1, 1, 0, 0, 0, 1 },
			{ 50, -50, -50, 0, 0, -1, 1, 0, 0, 1, 1 },

			// Right
			{ 50, 50, -50, 1, 0, 0, 0, 0, 1, 0, 0 },
			{ 50, 50, 50, 1, 0, 0, 0, 0, 1, 1, 0 },
			{ 50, -50, -50, 1, 0, 0, 0, 0, 1, 0, 1 },
			{ 50, -50, 50, 1, 0, 0, 0, 0, 1, 1, 1 },

			// Up
			{ 50, 50, -50, 0, 1, 0, -1, 0, 0, 0, 0 },
			{ -50, 50, -50, 0, 1, 0, -1, 0, 0, 1, 0 },
			{ 50, 50, 50, 0, 1, 0, -1, 0, 0, 0, 1 },
			{ -50, 50, 50, 0, 1, 0, -1, 0, 0, 1, 1 },

			// Down
			{ -50, -50, -50, 0, -1, 0, 1, 0, 0, 0, 0 },
			{ 50, -50, -50, 0, -1, 0, 1, 0, 0, 1, 0 },
			{ -50, -50, 50, 0, -1, 0, 1, 0, 0, 0, 1 },
			{ 50, -50, 50, 0, -1, 0, 1, 0, 0, 1, 1 }
		};

		const std::vector<uint32_t> indexList =
		{
			// Front
			0, 3, 2,
			0, 1, 3,

			// Left
			4, 7, 6,
			4, 5, 7,

			// Back
			8, 11, 10,
			8, 9, 11,

			// Right
			12, 15, 14,
			12, 13, 15,

			// Up
			16, 19, 18,
			16, 17, 19,

			// Down
			20, 23, 22,
			20, 21, 23
		};

		AssetManager::CreateMemoryOnlyAssetWithName<Mesh>("Cube - Built-In", vertexList, indexList);
	}

	void MeshFactory::CreateSphere()
	{
		std::vector<Vertex> vertexList;
		std::vector<uint32_t> indexList;

		const unsigned stacks = 16;
		const unsigned sectors = 32;

		const float radius = 50.0f;
		const float lengthInv = 1.0f / radius;

		const float stackStep = CU::Math::Pi / stacks;
		const float sectorStep = CU::Math::Tau / sectors;

		for (int i = 0; i <= stacks; i++)
		{
			for (int j = 0; j <= sectors; j++)
			{
				const float stackAngle = CU::Math::HalfPi - i * stackStep;
				const float sectorAngle = j * sectorStep;

				const float x = radius * cosf(stackAngle) * cosf(sectorAngle);
				const float y = radius * sinf(stackAngle);
				const float z = radius * cosf(stackAngle) * sinf(sectorAngle);

				const float nx = x * lengthInv;
				const float ny = y * lengthInv;
				const float nz = z * lengthInv;

				CU::Vector3f arbitrary = CU::Vector3f::Up;  // NOTE: This may be wrong at some points
				const CU::Vector3f tangent = CU::Vector3f(nx, ny, nz).Cross(arbitrary).GetNormalized();

				const float tx = tangent.x;
				const float ty = tangent.y;
				const float tz = tangent.z;

				const float u = static_cast<float>(j) / sectors;
				const float v = static_cast<float>(i) / stacks;

				vertexList.push_back(Vertex(x, y, z, nx, ny, nz, tx, ty, tz, u, v));
			}
		}

		int v1;
		int v2;

		for (int i = 0; i < stacks; ++i)
		{
			v1 = i * (sectors + 1);
			v2 = v1 + sectors + 1;

			for (int j = 0; j < sectors; ++j, ++v1, ++v2)
			{
				if (i != 0)
				{
					indexList.push_back(v1);
					indexList.push_back(v1 + 1);
					indexList.push_back(v2);
				}

				if (i != stacks - 1)
				{
					indexList.push_back(v1 + 1);
					indexList.push_back(v2 + 1);
					indexList.push_back(v2);
				}
			}
		}

		AssetManager::CreateMemoryOnlyAssetWithName<Mesh>("Sphere - Built-In", vertexList, indexList);
	}

	void MeshFactory::CreateCapsule()
	{
	}

	void MeshFactory::CreateCylinder()
	{
	}

	void MeshFactory::CreateQuad()
	{
		std::vector<Vertex> vertexList =
		{
			{ -50, 50, 0, 0, 0, -1, 1, 0, 0, 0, 0 },
			{ 50, 50, 0, 0, 0, -1, 1, 0, 0, 1, 0 },
			{ -50, -50, 0, 0, 0, -1, 1, 0, 0, 0, 1 },
			{ 50, -50, 0, 0, 0, -1, 1, 0, 0, 1, 1 }
		};

		std::vector<uint32_t> indexList =
		{
			0, 1, 3,
			0, 3, 2
		};

		AssetManager::CreateMemoryOnlyAssetWithName<Mesh>("Quad - Built-In", vertexList, indexList);
	}

	void MeshFactory::CreatePlane()
	{
		std::vector<Vertex> vertexList =
		{
			{ -100, 0, 100, 0, 1, 0, 1, 0, 0, 0, 0 },
			{ 100, 0, 100, 0, 1, 0, 1, 0, 0, 1, 0 },
			{ -100, 0, -100, 0, 1, 0, 1, 0, 0, 0, 1 },
			{ 100, 0, -100, 0, 1, 0, 1, 0, 0, 1, 1 }
		};

		std::vector<uint32_t> indexList =
		{
			0, 1, 3,
			0, 3, 2
		};

		AssetManager::CreateMemoryOnlyAssetWithName<Mesh>("Plane - Built-In", vertexList, indexList);
	}
}
