#pragma once

#include "Event.h"

namespace Zahra
{

	class ZAHRA_API WindowResizeEvent : public Event
	{
	public:

		WindowResizeEvent(unsigned int width, unsigned int height)
		: m_Width(width), m_Height(height) {}

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream stream;
			stream << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return stream.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)


	private:

		unsigned int m_Width;
		unsigned int m_Height;

	};

	class ZAHRA_API WindowCloseEvent : public Event
	{
	public:

		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class ZAHRA_API AppTickEvent : public Event
	{
	public:

		AppTickEvent() = default;

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class ZAHRA_API AppUpdateEvent : public Event
	{
	public:

		AppUpdateEvent() = default;

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class ZAHRA_API AppRenderEvent : public Event
	{
	public:

		AppRenderEvent() = default;

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

}