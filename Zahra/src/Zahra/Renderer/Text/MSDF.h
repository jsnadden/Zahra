#pragma once

#undef INFINITE // avoid clash with unnecessary windows definition
#include "msdf-atlas-gen.h"
#include "msdfgen.h"
#include "FontGeometry.h"
#include "GlyphGeometry.h"

#include <vector>

namespace Zahra
{
	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> GlyphGeometries;
		msdf_atlas::FontGeometry FontGeometry;
	};

}
