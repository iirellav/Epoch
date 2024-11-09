#pragma once
#include "Epoch/Assets/Asset.h"

namespace Epoch
{
	class MeshFactory
	{
	public:
		static void CreateCube();
		static void CreateSphere();
		static void CreateCapsule();
		static void CreateCylinder();
		static void CreateQuad();
		static void CreatePlane();
	};
}
