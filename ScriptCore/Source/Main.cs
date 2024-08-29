using System;
using System.Runtime.CompilerServices;

namespace Zahra
{
	public class Example
	{
		public float FloatVar { get; set; }
		public Example()
		{
			InternalCalls.NativeLog("Constructor called", 69);
		}

		~Example()
		{
			InternalCalls.NativeLog("Destructor called", 420);
		}

		public void Hello()
		{
			System.Console.WriteLine("Hello, world!");
		}

		public void Hello(int n)
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
		internal  extern static void NativeLog(string text, int parameter);

	}

}

