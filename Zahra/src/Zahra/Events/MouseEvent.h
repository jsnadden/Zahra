#pragma once

#include "Event.h"

namespace Zahra
{

	class ZAHRA_API MouseMovedEvent : public Event
	{
	public:

		MouseMovedEvent(float x, float y)
		: m_MouseX(x), m_MouseY(y) {}

		inline float GetMouseX() const { return m_MouseX; }
		inline float GetMouseY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream stream;
			stream << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return stream.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	
	private:

		float m_MouseX;
		float m_MouseY;

	};

	class ZAHRA_API MouseScrolledEvent : public Event
	{
	public:

		MouseScrolledEvent(float x, float y)
			: m_OffsetX(x), m_OffsetY(y) {}

		inline float GetOffsetX() const { return m_OffsetX; }
		inline float GetOffsetY() const { return m_OffsetY; }

		std::string ToString() const override
		{
			std::stringstream stream;
			stream << "MouseScrolledEvent: " << m_OffsetX << ", " << m_OffsetY;
			return stream.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)


	private:

		float m_OffsetX;
		float m_OffsetY;

	};

	class ZAHRA_API MouseButtonEvent : public Event
	{
	public:

		inline int GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	protected:

		MouseButtonEvent(int button) : m_Button(button) {}

		int m_Button;

	};

	class ZAHRA_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream stream;
			stream << "MouseButtonPressedEvent: " << m_Button;
			return stream.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class ZAHRA_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream stream;
			stream << "MouseButtonReleasedEvent: " << m_Button;
			return stream.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}