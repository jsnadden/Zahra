#pragma once

#include "Zahra/Scripting/ScriptEngine.h"

namespace Zahra
{
	class ScriptGlue
	{
	public:
		static void RegisterComponentTypes(MonoImage* assemblyImage);
		static void RegisterFunctions();
	};
}
