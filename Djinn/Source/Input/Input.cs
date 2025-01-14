
namespace Djinn
{
	public static class Input
	{
		public static bool IsKeyDown(KeyCode keyCode)
		{
			return Zahra.Input_IsKeyDown(keyCode);
		}

		public static bool IsMouseButtonPressed(MouseCode mouseCode)
		{
			return Zahra.Input_IsMouseButtonPressed(mouseCode);
		}

		public static void GetMousePosition(out float x, out float y)
		{
			Zahra.Input_GetMousePos(out x, out y);
		}

		public static float GetMouseX()
		{
			return Zahra.Input_GetMouseX();
		}

		public static float GetMouseY()
		{
			return Zahra.Input_GetMouseY();
		}
	}
}
