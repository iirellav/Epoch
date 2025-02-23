#pragma once
#include "Epoch/Math/AABB.h"

namespace Epoch
{
	struct MeshFile
	{
		struct Metadata
		{
			AABB boundingBox;

			uint64_t nodeArrayOffset;
			uint64_t nodeArraySize;

			uint64_t submeshArrayOffset;
			uint64_t submeshArraySize;

			uint64_t vertexBufferOffset;
			uint64_t vertexBufferSize;

			uint64_t indexBufferOffset;
			uint64_t indexBufferSize;
		};

		struct FileHeader
		{
			const char HEADER[4] = { 'E','P','M','F' };
			uint32_t version = 1;
		};

		FileHeader header;
		Metadata data;
	};
}
