#pragma once

#include "zpch.h"

#include "Zahra/Core/Base.h"
#include "Zahra/Events/Event.h"


namespace Zahra
{
	struct WindowProperties
	{
		std::string Title;

		uint32_t Width = 1600;
		uint32_t Height = 900;

		WindowProperties(std::string title = "Zahra_App") : Title(title) {}

	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual std::pair<uint32_t, uint32_t> GetPosition() const = 0;
		virtual void Move(uint32_t x, uint32_t y) = 0;

		enum WindowState { Default=0, Minimised, Maximised };

		virtual WindowState GetState() const = 0;
		virtual void SetState(WindowState state) = 0;

		virtual bool IsFullscreen() const = 0;
		virtual void SetFullscreen(bool enabled) = 0;

		// TODO: vsync property might better belong in GraphicsContext or elsewhere in the renderer stuff?
		virtual bool IsVSync() const = 0;
		virtual void SetVSync(bool enabled) = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;	

		virtual void ReadConfig() = 0;
		virtual void WriteConfig() = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProperties& props = WindowProperties());
	};

}
