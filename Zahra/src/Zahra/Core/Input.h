#pragma once

#include "Zahra/Core/Defines.h"
#include "Zahra/Core/KeyCodes.h"
#include "Zahra/Core/MouseCodes.h"

namespace Zahra
{

	
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode keycode);

		static bool IsMouseButtonPressed(MouseCode button);
		static std::pair<float, float> GetMousePos();
		static float GetMouseX();
		static float GetMouseY();

	};

}
