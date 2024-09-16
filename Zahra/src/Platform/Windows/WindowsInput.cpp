#include "zpch.h"

#include "Zahra/Core/Input.h"
#include "Zahra/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Zahra
{
	bool Input::IsKeyPressed(KeyCode keycode)
	{
		auto window = Application::Get().GetWindow().GetWindowHandle();
		auto state = glfwGetKey(window, static_cast<int32_t>(keycode));
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		auto window = Application::Get().GetWindow().GetWindowHandle();
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePos()
	{
		auto window = Application::Get().GetWindow().GetWindowHandle();
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		return std::make_pair((float)x,(float)y);
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePos();
		return x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePos();
		return y;
	}
}

