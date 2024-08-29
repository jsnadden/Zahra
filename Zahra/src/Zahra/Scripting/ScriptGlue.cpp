#include "zpch.h"
#include "ScriptGlue.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

namespace Zahra
{
	namespace InternalCalls
	{
		static void NativeLog(MonoString* arg1)
		{
			char* myString = mono_string_to_utf8(arg1);
			Z_CORE_INFO("From C#: {}", myString);
			mono_free(myString); // NOTE: manually free things mono has allocated for us!
		}
	}

#define Z_REGISTER_INTERNAL_CALL(name) mono_add_internal_call("Zahra.InternalCalls::"#name, (void*)InternalCalls::name);

	void ScriptGlue::RegisterFunctions()
	{
		Z_REGISTER_INTERNAL_CALL(NativeLog);

	}
	
}

