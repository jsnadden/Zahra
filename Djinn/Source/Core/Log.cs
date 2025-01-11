
namespace Djinn
{
	public static class Log
	{
		public static void Trace(string msg)
		{
			Zahra.Log_Trace(msg);
		}

		public static void Info(string msg)
		{
			Zahra.Log_Info(msg);
		}

		public static void Warn(string msg)
		{
			Zahra.Log_Warn(msg);
		}

		public static void Error(string msg)
		{
			Zahra.Log_Error(msg);
		}

		public static void Critical(string msg)
		{
			Zahra.Log_Critical(msg);
		}
	}
}
