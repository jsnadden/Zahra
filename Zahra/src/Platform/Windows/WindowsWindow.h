#pragma once

#include "Zahra/Core/Window.h"
#include "Zahra/Renderer/RendererContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Zahra
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& props);
		virtual ~WindowsWindow();

		virtual void PollEvents() override;

		inline uint32_t GetWidth() const override { return m_WindowData.Rectangle.Width; }
		inline uint32_t GetHeight() const override { return m_WindowData.Rectangle.Height; }
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual std::pair<uint32_t, uint32_t> GetPosition() const override;
		virtual void Move(uint32_t x, uint32_t y) override;

		virtual WindowState GetState() const override { return m_WindowData.State;  }
		virtual void SetState(WindowState state) override;

		virtual bool IsFullscreen() const override;
		virtual void SetFullscreen(bool enabled) override;

		inline void SetEventCallback(const EventCallbackFn& callback) override { m_WindowData.EventCallback = callback; };

		virtual void WriteConfig() override;
		virtual void ReadConfig() override;

		virtual Ref<RendererContext> GetRendererContext() override { return m_Context; }

		inline virtual GLFWwindow* GetWindowHandle() const override { return m_Window; };

	private:

		virtual void Init(const WindowProperties& props);
		virtual void Shutdown();

		GLFWwindow* m_Window;

		Ref<RendererContext> m_Context;

		struct WindowRectangle
		{
			uint32_t Width = 1600;
			uint32_t Height = 900;
			uint32_t XPosition = 100;
			uint32_t YPosition = 100;
		};

		struct Win32WindowData
		{
			std::string Title;
			
			WindowRectangle Rectangle;
			WindowRectangle RectangleCache;

			WindowState State = WindowState::Default;

			bool Fullscreen = false;
			bool VSync = false;

			EventCallbackFn EventCallback;
		};

		Win32WindowData m_WindowData;
	};
}
