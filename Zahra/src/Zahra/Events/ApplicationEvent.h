#pragma once

#include "Event.h"

namespace Zahra
{

	class WindowResizedEvent : public Event
	{
	public:

		WindowResizedEvent(unsigned int width, unsigned int height)
		: m_Width(width), m_Height(height) {}

		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream stream;
			stream << "WindowResizedEvent: " << m_Width << ", " << m_Height;
			return stream.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)


	private:

		unsigned int m_Width;
		unsigned int m_Height;

	};

	class WindowMovedEvent : public Event
	{
	public:

		WindowMovedEvent(float x, float y)
			: m_PositionX(x), m_PositionY(y) {}

		float GetPositionX() const { return m_PositionX; }
		float GetPositionY() const { return m_PositionY; }

		std::string ToString() const override
		{
			std::stringstream stream;
			stream << "WindowMovedEvent: " << m_PositionX << ", " << m_PositionY;
			return stream.str();
		}

		EVENT_CLASS_TYPE(WindowMoved)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)


	private:

		float m_PositionX;
		float m_PositionY;

	};

	class WindowMinimisedEvent : public Event
	{
	public:

		WindowMinimisedEvent(int minimised)
			: m_Minimised((bool)minimised) {}

		float Minimised() const { return m_Minimised; }

		std::string ToString() const override
		{
			std::stringstream stream;
			stream << "WindowMinimisedEvent: " << (m_Minimised ? "minimised" : "restored");
			return stream.str();
		}

		EVENT_CLASS_TYPE(WindowMinimised)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)


	private:
		bool m_Minimised;

	};

	class WindowMaximisedEvent : public Event
	{
	public:

		WindowMaximisedEvent(int maximised)
			: m_Maximised((bool)maximised) {}

		float Maximised() const { return m_Maximised; }

		std::string ToString() const override
		{
			std::stringstream stream;
			stream << "WindowMaximisedEvent: " << (m_Maximised ? "maximised" : "restored");
			return stream.str();
		}

		EVENT_CLASS_TYPE(WindowMaximised)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)


	private:
		bool m_Maximised;

	};

	class WindowClosedEvent : public Event
	{
	public:

		WindowClosedEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppTickEvent : public Event
	{
	public:

		AppTickEvent() = default;

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event
	{
	public:

		AppUpdateEvent() = default;

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event
	{
	public:

		AppRenderEvent() = default;

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

}
