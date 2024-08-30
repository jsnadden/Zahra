using System;
using System.Runtime.CompilerServices;

namespace Zahra
{

	public class Entity
	{
		protected Entity() { GUID = 0; Console.WriteLine("guidless entity constructor"); }
		internal Entity(ulong guid) { GUID = guid; Console.WriteLine("guidful entity constructor"); }

		public readonly ulong GUID;

		public Vector3 Translation
		{
			get
			{
				InternalCalls.Entity_GetTranslation(GUID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.Entity_SetTranslation(GUID, ref value);
			}

		}

	}

	public static class InternalCalls
	{ 
		// Import C++ method into C#
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal  extern static void NativeLog(string text);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_GetTranslation(ulong guid, out Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetTranslation(ulong guid, ref Vector3 translation);
	}

}

