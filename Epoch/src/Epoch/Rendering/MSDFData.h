#pragma once
#undef INFINITE
#include <msdf-atlas-gen.h>
#include <msdf-atlas-gen/msdfgen/msdfgen.h>

#include <vector>

namespace Epoch
{
	struct MSDFData
	{
		msdf_atlas::FontGeometry fontGeometry;
		std::vector<msdf_atlas::GlyphGeometry> glyphs;
	};
}

