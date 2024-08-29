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

		public void Test()
		{
			System.Console.WriteLine("Hello, world!");
		}

		public void Test(int n)
		{
			for (int i = 0; i < n; i++)
			{
				System.Console.WriteLine("Hello, world!");

			}
		}

	}

	public static class InternalCalls
	{ 
		// Import C++ method into C#
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal  extern static void NativeLog(string text);

	}

}

