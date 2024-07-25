#pragma once

#include "zpch.h"

#include "Zahra/Core/Base.h"
#include "Zahra/Events/Event.h"


namespace Zahra
{
	struct WindowProperties
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProperties(const std::string& title = "Zahra", uint32_t width = 1600, uint32_t height = 900)
			: Title(title), Width(width), Height(height) {}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

		// TODO: add functions to get/set window position (and other
		// statuses e.g. maximised), and hook this into some kind of
		// AppConfig class (with serialisation to a config.ini, or 
		// maybe .yaml?). I'm sick of repositioning the window!!

		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProperties& props = WindowProperties());
	};

}
