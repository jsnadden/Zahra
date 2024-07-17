#include "EditorLayer.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include "Zahra/Scene/SceneSerialiser.h"
#include "Zahra/Utils/PlatformUtils.h"

namespace Zahra
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
	}

	void EditorLayer::OnAttach()
	{
		FramebufferSpecification framebufferSpec;
		framebufferSpec.Width = 1280;
		framebufferSpec.Height = 720; // These will be overwritten by the ImGui viewport window

		m_Framebuffer = Framebuffer::Create(framebufferSpec);

		m_ActiveScene = CreateRef<Scene>();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(float dt)
	{
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// REACT TO RESIZED VIEWPORT
		{
			FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f // framebuffer requires positive dimensions
				&& (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
			{
				m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAPTURE DRAW CALLS IN FRAMEBUFFER
		{
			Renderer2D::ResetStats();
			
			m_Framebuffer->Bind();
			{
				RenderCommand::SetClearColour(glm::make_vec4(m_ClearColour));
				RenderCommand::Clear();

				m_ActiveScene->OnUpdate(dt);
			}
			m_Framebuffer->Unbind();
			
		}

	}

	void EditorLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
	}

	void EditorLayer::OnImGuiRender()
	{
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MAIN MENU
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New", "Ctrl+N")) NewScene();

					if (ImGui::MenuItem("Open...", "Ctrl+O")) OpenSceneFile();

					ImGui::Separator();

					if (ImGui::MenuItem("Save", "Ctrl+S")) SaveSceneFile();

					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) SaveAsSceneFile();

					ImGui::Separator();

					// TODO: check for unsaved content, warn user with an imgui popup (would you like to save?)
					if (ImGui::MenuItem("Exit"))
					{
						Application::Get().Exit();
					}

					ImGui::EndMenu();
				}

			}
			ImGui::EndMainMenuBar();

		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// WINDOW DOCKSPACE
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// VIEWPORT
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
			ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoCollapse);

			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();

			Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			size_t framebufferTextureID = m_Framebuffer->GetColourAttachmentRendererID();
			ImGui::Image((void*)framebufferTextureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2( 0, 1 ), ImVec2( 1, 0 ));
			
			ImGui::End();
			ImGui::PopStyleVar();
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SCENE HIERARCHY & PROPERTIES PANELS
		m_SceneHierarchyPanel.OnImGuiRender();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// STATS WINDOW
		{
			ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoCollapse);

			ImGui::Text("Quads: %u", Renderer2D::GetStats().QuadCount);
			ImGui::Text("Draw calls: %u", Renderer2D::GetStats().DrawCalls);

			ImGui::End();
		}

	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		return false;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenSceneFile()
	{
		std::optional<std::string> filepath = FileDialogs::OpenFile(m_FileTypesFilter);
		if (filepath)
		{
			NewScene();

			std::string sceneName = filepath->substr(filepath->find_last_of("/\\") + 1);
			m_ActiveScene->SetName(sceneName);

			SceneSerialiser serialiser(m_ActiveScene);
			serialiser.DeserialiseYaml(*filepath);
		}

		// TODO: report/display success of file open
	}

	void EditorLayer::SaveSceneFile()
	{
		

		// TODO: report/display success of file save
	}

	void EditorLayer::SaveAsSceneFile()
	{
		std::optional<std::string> filepath = FileDialogs::SaveFile(m_FileTypesFilter);
		if (filepath)
		{
			std::string sceneName = filepath->substr(filepath->find_last_of("/\\") + 1);
			m_ActiveScene->SetName(sceneName);

			SceneSerialiser serialiser(m_ActiveScene);
			serialiser.SerialiseYaml(*filepath);
		}

		// TODO: report/display success of file save
	}


}

