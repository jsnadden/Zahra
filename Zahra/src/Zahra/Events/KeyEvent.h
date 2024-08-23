#pragma once
#include "Event.h"

#include "Zahra/Core/Input.h"

namespace Zahra
{

	class KeyEvent : public Event
	{
	public:

		KeyCode GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:

		KeyEvent(KeyCode keycode) : m_KeyCode(keycode) {}

		KeyCode m_KeyCode;

	};

	class KeyPressedEvent : public KeyEvent
	{
	public:

		KeyPressedEvent(KeyCode keycode, bool repeat)
			: KeyEvent(keycode), m_Repeated(repeat) {}

		bool Repeated() const { return m_Repeated; }

		std::string ToString() const override
		{
			std::stringstream stream;

			stream << "KeyPressedEvent: " << m_KeyCode << (m_Repeated ? " (repeat)" : "");

			return stream.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:

		bool m_Repeated;
	};
	
	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream stream;

			stream << "KeyReleasedEvent: " << m_KeyCode;

			return stream.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:

		KeyTypedEvent(KeyCode keycode) : KeyEvent(keycode) {}


		std::string ToString() const override
		{
			std::stringstream stream;

			stream << "KeyTypedEvent: " << m_KeyCode;

			return stream.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)

	};
}
