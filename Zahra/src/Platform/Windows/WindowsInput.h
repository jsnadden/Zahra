#pragma once

#include "Zahra/Core/Input.h"

namespace Zahra
{
	class WindowsInput : public Input
	{
	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;

		virtual bool IsMouseButtonPressedImpl(int button) override;
		// Should implement a Vec2 struct first, then fuse the following into a single GetMousePos
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;

	private:

	};
}

