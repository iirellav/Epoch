#pragma once
#include <vector>
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	struct Vertex;

	class MeshFactory
	{
	public:
		static std::pair<std::vector<Vertex>, std::vector<uint32_t>> CreateCube();
		static std::pair<std::vector<Vertex>, std::vector<uint32_t>> CreateSphere();
		//static std::pair<std::vector<Vertex>, std::vector<uint32_t>> CreateCapsule();
		//static std::pair<std::vector<Vertex>, std::vector<uint32_t>> CreateCylinder();
		static std::pair<std::vector<Vertex>, std::vector<uint32_t>> CreateQuad();
		static std::pair<std::vector<Vertex>, std::vector<uint32_t>> CreatePlane();
	};
}
