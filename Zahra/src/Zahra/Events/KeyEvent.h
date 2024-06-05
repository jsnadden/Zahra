#pragma once

#include "Event.h"

namespace Zahra
{

	class ZAHRA_API KeyEvent : public Event
	{
	public:

		inline int GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:

		KeyEvent(int keycode) : m_KeyCode(keycode) {}

		int m_KeyCode;

	};

	class ZAHRA_API KeyPressedEvent : public KeyEvent
	{
	public:

		KeyPressedEvent(int keycode, int repeatcount)
			: KeyEvent(keycode), m_RepeatCount(repeatcount) {}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream stream;

			stream << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";

			return stream.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:

		int m_RepeatCount;
	};
	
	class ZAHRA_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream stream;

			stream << "KeyReleasedEvent: " << m_KeyCode;

			return stream.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};
}