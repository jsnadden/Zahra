#include "zpch.h"

#include "Zahra/Core/Input.h"
#include "Zahra/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Zahra
{
	bool Input::IsKeyPressed(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	float Input::GetMouseX()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double x;
		glfwGetCursorPos(window, &x, NULL);
		return (float)x;
	}

	float Input::GetMouseY()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double y;
		glfwGetCursorPos(window, NULL, &y);
		return (float)y;
	}
}

