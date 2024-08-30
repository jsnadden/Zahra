#include "zpch.h"
#include "ScriptGlue.h"

#include "Zahra/Scripting/ScriptEngine.h"

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

		static void Entity_GetTranslation(ZGUID guid, glm::vec3* out_translation)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			
			*out_translation = entity.GetComponents<TransformComponent>().Translation;
		}

		static void Entity_SetTranslation(ZGUID guid, glm::vec3* translation)
		{
			Entity entity = ScriptEngine::GetEntity(guid);

			entity.GetComponents<TransformComponent>().Translation = *translation;
		}

		static bool Input_IsKeyDown(KeyCode key)
		{
			return Input::IsKeyPressed(key);
		}

	}

#define Z_REGISTER_INTERNAL_CALL(name) mono_add_internal_call("Zahra.InternalCalls::"#name, (void*)InternalCalls::name);

	void ScriptGlue::RegisterFunctions()
	{
		///////////////////////////////////////////////////////////////////////////////////////////////////
		// ENGINE CORE
		{
			// Input
			Z_REGISTER_INTERNAL_CALL(Input_IsKeyDown);

			// Logging
			Z_REGISTER_INTERNAL_CALL(NativeLog);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// ECS
		{
			// TransformComponent
			Z_REGISTER_INTERNAL_CALL(Entity_GetTranslation);
			Z_REGISTER_INTERNAL_CALL(Entity_SetTranslation);
		}

	}
	
}

