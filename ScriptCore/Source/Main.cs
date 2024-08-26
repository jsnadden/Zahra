using System;
using System.Runtime.CompilerServices;

namespace Zahra
{
	public class Main
	{
		public float FloatVar { get; set; }
		public Main()
		{
			Console.WriteLine("Main constructed.");
		}

		~Main()
		{
			Console.WriteLine("Main destructed.");
		}

		public void PrintMessage()
		{
			Console.WriteLine("Printing default message.");
		}

		public void PrintNativeLog()
		{
			NativeLog("boobs", 58008);
		}
		
		// Import C++ method into C#
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		extern static void NativeLog(string text, int parameter);

	}

}

