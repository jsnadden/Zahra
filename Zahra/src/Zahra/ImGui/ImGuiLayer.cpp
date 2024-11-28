#include "zpch.h"
#include "ImGuiLayer.h"

#include "Platform/OpenGL/OpenGLImGuiLayer.h"
#include "Platform/Vulkan/VulkanImGuiLayer.h"
#include "Zahra/Renderer/Renderer.h"

#include <imgui.h>

namespace Zahra
{

	ImGuiLayer* ImGuiLayer::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:    return new OpenGLImGuiLayer("OpenGL_ImGui_Layer");
		case RendererAPI::API::Vulkan:    return new VulkanImGuiLayer("Vulkan_ImGui_Layer");
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}

	void ImGuiLayer::SetColourTheme()
	{
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
