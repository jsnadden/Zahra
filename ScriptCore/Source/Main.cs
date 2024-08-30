using System;
using System.Runtime.CompilerServices;

namespace Zahra
{
	public class Entity
	{
		public float FloatVar { get; set; }
		public Entity()
		{
			InternalCalls.NativeLog("Entity constructed");
		}

		~Entity()
		{
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

	}

}

