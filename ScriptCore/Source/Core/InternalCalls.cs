using System.Runtime.CompilerServices;

namespace Djinn
{
	public static class InternalCalls
	{
		///////////////////////////////////////////////////////////////////////////////////////////////////
		// ENGINE CORE

		// Input
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode key);

		// Logging
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog(string text);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// ECS

		// TransformComponent
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_GetTranslation(ulong guid, out Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetTranslation(ulong guid, ref Vector3 translation);
	}
}
