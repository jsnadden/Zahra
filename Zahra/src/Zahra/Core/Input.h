#pragma once

#include "Zahra/Core/Base.h"

namespace Zahra
{

	
	class Input
	{
	public:
		static bool IsKeyPressed(int keycode);

		static bool IsMouseButtonPressed(int button);
		static float GetMouseX();
		static float GetMouseY();

	};

}