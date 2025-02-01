#include "zpch.h"
#include "ImGuiLayer.h"

#include "Platform/Vulkan/VulkanImGuiLayer.h"
#include "Zahra/Core/Application.h"
#include "Zahra/Renderer/Renderer.h"

#include <imgui.h>

namespace Zahra
{
	namespace ImGuiUtils
	{
		static ImVec4 ColourCorrect(const ImVec4& srgb, bool ignore = false)
		{
			if (ignore)
				return srgb;

			static auto& conversionFormula = [](float u)
				{
					return u < 0.04045f ?
						u / 12.92f :
						std::pow((u + 0.055f) / 1.055f, 2.4f);
				};

			ImVec4 linear{};
			linear.x = conversionFormula(srgb.x);
			linear.y = conversionFormula(srgb.y);
			linear.z = conversionFormula(srgb.z);
			linear.w = conversionFormula(srgb.w);

			return linear;
		}
	}

	ImGuiLayer* ImGuiLayer::GetOrCreate()
	{
		ImGuiLayer* layer = Application::Get().GetImGuiLayer();

		if (layer)
			return layer;

		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
			case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return new VulkanImGuiLayer("Vulkan_ImGui_Layer");
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}

	void ImGuiLayer::SetColourTheme()
	{
		auto& colours = ImGui::GetStyle().Colors;

		// Text
		colours[ImGuiCol_Text]						= ImGuiUtils::ColourCorrect(ImVec4{ .98f, .95f, .84f, 1.00f });
		colours[ImGuiCol_TextSelectedBg]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.00f });
		colours[ImGuiCol_TextDisabled]				= ImGuiUtils::ColourCorrect(ImVec4{ .75f, .68f, .58f, 1.00f });

		// Window and menu backgrounds
		colours[ImGuiCol_WindowBg]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });		// Background of normal windows
		colours[ImGuiCol_ChildBg]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });		// Background of child windows
		colours[ImGuiCol_PopupBg]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f });		// Background of popups, menus, tooltips windows
		colours[ImGuiCol_NavWindowingHighlight]		= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });		// Highlight window when using CTRL+TAB
		colours[ImGuiCol_NavWindowingDimBg]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.1f, 0.1f, 0.1f, .5f });			// Darken/colorize entire screen behind the CTRL+TAB window list, when active
		colours[ImGuiCol_ModalWindowDimBg]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.1f, 0.1f, 0.1f, .5f });			// Darken/colorize entire screen behind a modal window, when one is active

		// Dockspace
		colours[ImGuiCol_DockingEmptyBg]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.18f, 0.18f, 0.18f, 1.0f });		// Background color for empty node (e.g. CentralNode with no window docked into it)
		colours[ImGuiCol_DockingPreview]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });		// Preview overlay color when about to docking something

		// Scroll bars
		colours[ImGuiCol_ScrollbarBg]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });
		colours[ImGuiCol_ScrollbarGrab]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f });
		colours[ImGuiCol_ScrollbarGrabHovered]		= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });
		colours[ImGuiCol_ScrollbarGrabActive]		= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });

		// Tabs
		colours[ImGuiCol_Tab]						= ImGuiUtils::ColourCorrect(ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f });		// Tab background, when tab-bar is focused & tab is unselected
		colours[ImGuiCol_TabHovered]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });		// Tab background, when hovered
		colours[ImGuiCol_TabSelected]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f });		// Tab background, when tab-bar is focused & tab is selected
		colours[ImGuiCol_TabSelectedOverline]		= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });		// Tab horizontal overline, when tab-bar is focused & tab is selected
		colours[ImGuiCol_TabDimmed]					= ImGuiUtils::ColourCorrect(ImVec4{ .14f, .14f, .14f, 1.0f });		// Tab background, when tab-bar is unfocused & tab is unselected
		colours[ImGuiCol_TabDimmedSelected]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });		// Tab background, when tab-bar is unfocused & tab is selected
		colours[ImGuiCol_TabDimmedSelectedOverline]	= ImGuiUtils::ColourCorrect(ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });		//..horizontal overline, when tab-bar is unfocused & tab is selected

		// Title
		colours[ImGuiCol_MenuBarBg]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f });
		colours[ImGuiCol_TitleBg]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f });
		colours[ImGuiCol_TitleBgActive]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
		colours[ImGuiCol_TitleBgCollapsed]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f });

		// Borders
		colours[ImGuiCol_DragDropTarget]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f });
		colours[ImGuiCol_Border]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f });		// Borders of menus and popups
		colours[ImGuiCol_BorderShadow]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f });		// Collapsed window edges
		colours[ImGuiCol_SeparatorHovered]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });
		colours[ImGuiCol_SeparatorActive]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f });
		colours[ImGuiCol_ResizeGrip]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f });
		colours[ImGuiCol_ResizeGripHovered]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });
		colours[ImGuiCol_ResizeGripActive]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f });

		// Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
		colours[ImGuiCol_Header]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f });
		colours[ImGuiCol_HeaderHovered]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });
		colours[ImGuiCol_HeaderActive]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f });

		// Widgets
		colours[ImGuiCol_Button]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f });
		colours[ImGuiCol_ButtonHovered]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });
		colours[ImGuiCol_ButtonActive]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.92f, 0.72f, 0.18f, 1.0f });
		colours[ImGuiCol_CheckMark]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });
		colours[ImGuiCol_SliderGrab]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f });
		colours[ImGuiCol_SliderGrabActive]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });

		// Background of checkbox, radio button, plot, slider, text input
		colours[ImGuiCol_FrameBg]					= ImGuiUtils::ColourCorrect(ImVec4{ 0.24f, 0.24f, 0.24f, 1.0f });
		colours[ImGuiCol_FrameBgHovered]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
		colours[ImGuiCol_FrameBgActive]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.18f, 0.18f, 0.18f, 1.0f });

		// Tables
		colours[ImGuiCol_TableHeaderBg]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });		// Table header background
		colours[ImGuiCol_TableBorderStrong]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });		// Table outer and header borders (prefer using Alpha=1.0 here)
		colours[ImGuiCol_TableBorderLight]			= ImGuiUtils::ColourCorrect(ImVec4{ 0.97f, 0.77f, 0.22f, 1.0f });		// Table inner borders (prefer using Alpha=1.0 here)
		colours[ImGuiCol_TableRowBg]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f });		// Table row background (even rows)
		colours[ImGuiCol_TableRowBgAlt]				= ImGuiUtils::ColourCorrect(ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });		// Table row background (odd rows)

	}

}
