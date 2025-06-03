#include "zpch.h"
#include "Font.h"

#undef INFINITE // winbase.h (windows) defines this somewhere, but msdfgen uses the token for something else, so we free it up
#include "msdf-atlas-gen.h"
#include "msdfgen.h"

namespace Zahra
{
	Font::Font(const std::filesystem::path& sourceFilepath)
	{

	}

}
