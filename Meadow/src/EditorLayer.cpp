#include "EditorLayer.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

#include "Zahra/Scene/SceneSerialiser.h"
#include "Zahra/Utils/PlatformUtils.h"
#include "Zahra/Maths/Maths.h"

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

					// TODO: warning popup: unsaved changes
					if (ImGui::MenuItem("Exit"))
					{
						Application::Get().Exit();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Tools"))
				{
					if (ImGui::MenuItem("Select", "Q")) m_GizmoType = -1;
					if (ImGui::MenuItem("Translate", "W")) m_GizmoType = 0;
					if (ImGui::MenuItem("Rotate", "E")) m_GizmoType = 1;
					if (ImGui::MenuItem("Scale", "R")) m_GizmoType = 2;
					
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

			Application::Get().GetImGuiLayer()->BlockEvents(false);

			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			size_t framebufferTextureID = m_Framebuffer->GetColourAttachmentRendererID();
			ImGui::Image((void*)framebufferTextureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2( 0, 1 ), ImVec2( 1, 0 ));
			
			// Transform Gizmos
			Entity selection = m_SceneHierarchyPanel.GetSelectedEntity();
			if (selection && m_GizmoType != -1)
			{
				auto cameraEntity = m_ActiveScene->GetActiveCamera();

				if (cameraEntity)
				{
					ImGuizmo::SetOrthographic(false); // TODO: make this work with orth cameras too!
					ImGuizmo::SetDrawlist();

					float windowWidth = (float)ImGui::GetWindowWidth();
					float windowHeight = (float)ImGui::GetWindowHeight();
					ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

					const auto& camera = cameraEntity.GetComponents<CameraComponent>().Camera;
					glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponents<TransformComponent>().GetTransform());
					const glm::mat4& cameraProjection = camera.GetProjection();

					auto& tc = selection.GetComponents<TransformComponent>();
					glm::mat4 transform = tc.GetTransform();

					ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
						(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

					if (ImGuizmo::IsUsing())
						Maths::DecomposeTransform(transform, tc.Translation,tc.EulerAngles, tc.Scale);			
										
				}

			}

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
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Keyboard shortcuts
		//if (event.GetRepeatCount() > 0) return false;

		bool ctrl = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

		switch (event.GetKeyCode())
		{
			case KeyCode::S:
			{
				if (ctrl && shift)
				{
					SaveAsSceneFile();
					return true;
				}
				else if (ctrl)
				{
					SaveSceneFile();
					return true;
				}

				break;
			}
			case KeyCode::N:
			{
				if (ctrl) 
				{
					NewScene();
					return true;
				}

				break;
			}
			case KeyCode::O:
			{
				if (ctrl) 
				{
					OpenSceneFile();
					return true;
				}

				break;
			}
			case KeyCode::Q:
			{
				m_GizmoType = -1;
				break;
			}
			case KeyCode::W:
			{
				m_GizmoType = 0;
				break;
			}
			case KeyCode::E:
			{
				m_GizmoType = 1;
				break;
			}
			case KeyCode::R:
			{
				m_GizmoType = 2;
				break;
			}
			default:
				break;
			}

		return false;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_CurrentFilePath = std::nullopt;
	}

	void EditorLayer::OpenSceneFile()
	{
		std::optional<std::string> filepath = FileDialogs::OpenFile(m_FileTypesFilter);
		if (filepath)
		{
			NewScene();

			std::string sceneName = filepath->substr(filepath->find_last_of("/\\") + 1);
			m_ActiveScene->SetName(sceneName);
			m_CurrentFilePath = filepath;

			SceneSerialiser serialiser(m_ActiveScene);
			serialiser.DeserialiseYaml(*m_CurrentFilePath);
		}

		// TODO: report/display success of file open
	}

	void EditorLayer::SaveSceneFile()
	{
		if (!m_CurrentFilePath)
		{
			SaveAsSceneFile();
		}
		else
		{
			SceneSerialiser serialiser(m_ActiveScene);
			serialiser.SerialiseYaml(*m_CurrentFilePath);
		}

		// TODO: report/display success of file save
	}

	void EditorLayer::SaveAsSceneFile()
	{
		std::optional<std::string> filepath = FileDialogs::SaveFile(m_FileTypesFilter);
		if (filepath)
		{
			std::string sceneName = filepath->substr(filepath->find_last_of("/\\") + 1);
			m_ActiveScene->SetName(sceneName);
			m_CurrentFilePath = filepath;

			SceneSerialiser serialiser(m_ActiveScene);
			serialiser.SerialiseYaml(*m_CurrentFilePath);
		}

		// TODO: report/display success of file save
	}


}

