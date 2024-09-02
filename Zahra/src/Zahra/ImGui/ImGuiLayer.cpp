#include "zpch.h"
#include "ImGuiLayer.h"

#include "Zahra/Core/Application.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <ImGuizmo.h>

// SHOULD EVENTUALLY REMOVE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Zahra
{
	ImGuiLayer::ImGuiLayer() : Layer("ImGui_Layer")
	{}

	ImGuiLayer::~ImGuiLayer()
	{}

	void ImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// TODO: write a font library so that I don't have to rely on imgui's internal font vector
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Resources\\Fonts\\Inter\\Inter-Regular.ttf", 18.0f); // font 0
		io.Fonts->AddFontFromFileTTF("Resources\\Fonts\\Inter\\Inter-Bold.ttf", 18.0f); // font 1

		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetColourTheme();

		Application& app = Application::Get();

		/////////////////////////////////////////////
		// TODO: make this platform-independent!!
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
		/////////////////////////////////////////////
	}

	void ImGuiLayer::OnDetach()
	{
		Z_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) && io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) && io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::Begin()
	{
		Z_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		Z_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}


	}

	// TODO: it would be nice to read/write colour themes (yaml, json, or even just ini),
	// and especially nice to design a menu popup for this!
	void ImGuiLayer::SetColourTheme()
	{
		// TODO: customise this default scheme, and figure out what the rest of the options are in ImGuiCol_
		auto& colours = ImGui::GetStyle().Colors;

		// Text
		colours[ImGuiCol_Text]				= ImVec4{ .98f, .95f, .84f, 1.00f };
		colours[ImGuiCol_TextSelectedBg]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.00f };
		colours[ImGuiCol_TextDisabled]		= ImVec4{ .75f, .68f, .58f, 1.00f };

		// Window and menu backgrounds
		colours[ImGuiCol_WindowBg]				= ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f }; // Background of normal windows
		colours[ImGuiCol_ChildBg]				= ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f }; // Background of child windows
		colours[ImGuiCol_PopupBg]				= ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f }; // Background of popups, menus, tooltips windows
		colours[ImGuiCol_NavWindowingHighlight]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f }; // Highlight window when using CTRL+TAB
		colours[ImGuiCol_NavWindowingDimBg]		= ImVec4{ 0.1f, 0.1f, 0.1f, .5f }; // Darken/colorize entire screen behind the CTRL+TAB window list, when active
		colours[ImGuiCol_ModalWindowDimBg]		= ImVec4{ 0.1f, 0.1f, 0.1f, .5f }; // Darken/colorize entire screen behind a modal window, when one is active

		// Dockspace
		colours[ImGuiCol_DockingEmptyBg] = ImVec4{ 0.18f, 0.18f, 0.18f, 1.0f }; // Background color for empty node (e.g. CentralNode with no window docked into it)
		colours[ImGuiCol_DockingPreview] = ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f }; // Preview overlay color when about to docking something

		// Scroll bars
		colours[ImGuiCol_ScrollbarBg]			= ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };
		colours[ImGuiCol_ScrollbarGrab]			= ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f };
		colours[ImGuiCol_ScrollbarGrabHovered]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };
		colours[ImGuiCol_ScrollbarGrabActive]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };

		// Tabs
		colours[ImGuiCol_Tab]						= ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f };	// Tab background, when tab-bar is focused & tab is unselected
		colours[ImGuiCol_TabHovered]				= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };	// Tab background, when hovered
		colours[ImGuiCol_TabSelected]				= ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };	// Tab background, when tab-bar is focused & tab is selected
		colours[ImGuiCol_TabSelectedOverline]		= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };	// Tab horizontal overline, when tab-bar is focused & tab is selected
		colours[ImGuiCol_TabDimmed]					= ImVec4{ .14f, .14f, .14f, 1.0f };	// Tab background, when tab-bar is unfocused & tab is unselected
		colours[ImGuiCol_TabDimmedSelected]			= ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };	// Tab background, when tab-bar is unfocused & tab is selected
		colours[ImGuiCol_TabDimmedSelectedOverline]	= ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };	//..horizontal overline, when tab-bar is unfocused & tab is selected

		// Title
		colours[ImGuiCol_MenuBarBg]			= ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };
		colours[ImGuiCol_TitleBg]			= ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };
		colours[ImGuiCol_TitleBgActive]		= ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
		colours[ImGuiCol_TitleBgCollapsed]	= ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };

		// Borders
		colours[ImGuiCol_DragDropTarget]	= ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f };
		colours[ImGuiCol_Border]			= ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f }; // Borders of menus and popups
		colours[ImGuiCol_BorderShadow]		= ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f }; // Collapsed window edges
		colours[ImGuiCol_SeparatorHovered]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };
		colours[ImGuiCol_SeparatorActive]	= ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f };
		colours[ImGuiCol_ResizeGrip]		= ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f };
		colours[ImGuiCol_ResizeGripHovered]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };
		colours[ImGuiCol_ResizeGripActive]	= ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f };

		// Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
		colours[ImGuiCol_Header]		= ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f };
		colours[ImGuiCol_HeaderHovered]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };
		colours[ImGuiCol_HeaderActive]	= ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f };

		// Widgets
		colours[ImGuiCol_Button]			= ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f };
		colours[ImGuiCol_ButtonHovered]		= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };
		colours[ImGuiCol_ButtonActive]		= ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f };
		colours[ImGuiCol_CheckMark]			= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };
		colours[ImGuiCol_SliderGrab]		= ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };
		colours[ImGuiCol_SliderGrabActive]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f };

		// Background of checkbox, radio button, plot, slider, text input
		colours[ImGuiCol_FrameBg]			= ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f };
		colours[ImGuiCol_FrameBgHovered]	= ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
		colours[ImGuiCol_FrameBgActive]		= ImVec4{ 0.18f, 0.18f, 0.18f, 1.0f };

		// Tables
		colours[ImGuiCol_TableHeaderBg]		= ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f }; // Table header background
		colours[ImGuiCol_TableBorderStrong]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f }; // Table outer and header borders (prefer using Alpha=1.0 here)
		colours[ImGuiCol_TableBorderLight]	= ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f }; // Table inner borders (prefer using Alpha=1.0 here)
		colours[ImGuiCol_TableRowBg]		= ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f }; // Table row background (even rows)
		colours[ImGuiCol_TableRowBgAlt]		= ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f }; // Table row background (odd rows)

	}


}
