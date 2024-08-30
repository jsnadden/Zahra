using System;
using System.Runtime.CompilerServices;

namespace Zahra
{

	public class Entity
	{
		public ulong GUID { get; set; }

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
				Translation = value;
			}

		}

		public Entity()
		{
			
			
			// TODO: comment this out eventually, it's just for debugging purposes
			InternalCalls.NativeLog("Entity constructed");
		}

		~Entity()
		{


			// TODO: comment this out eventually, it's just for debugging purposes
			InternalCalls.NativeLog("Entity destroyed");
		}

		public virtual void OnCreate() { }
		public virtual void OnUpdate(float dt) { }

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

