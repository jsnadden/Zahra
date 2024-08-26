using System;

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

		public void PrintCustomMessage(string message)
		{
			Console.WriteLine($"Printing custom message: {message}");
		}

		public void PrintInteger(int x)
		{
			Console.WriteLine($"Printing integer: {x}");
		}

	}

}

