#pragma once
#include <cstdint>

namespace Epoch
{
	constexpr uint32_t MaxInstanceCount = 1024 * 4;

	constexpr uint32_t MaxTextureSlots = 32;

	constexpr uint32_t MaxQuads = 512;
	constexpr uint32_t MaxQuadIndices = MaxQuads * 6;
	constexpr uint32_t MaxQuadVertices = MaxQuads * 4;

	constexpr uint32_t MaxLineIndices = 1024 * 3;
	constexpr uint32_t MaxLineVertices = 1024 * 2;
}
