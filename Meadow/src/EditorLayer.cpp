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
		: Layer("EditorLayer") {}

	void EditorLayer::OnAttach()
	{
		FramebufferSpecification framebufferSpec;
		framebufferSpec.AttachmentSpec = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH24STENCIL8 };
		framebufferSpec.Width = 1280;
		framebufferSpec.Height = 720; // These will be overwritten by the ImGui viewport window

		m_Framebuffer = Framebuffer::Create(framebufferSpec);

		m_ActiveScene = CreateRef<Scene>();

		m_EditorCamera = EditorCamera(.5f, 1.78f, .1f, 1000.f);

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
				m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			}
		}

		if (m_ViewportHovered)
		{
			m_EditorCamera.OnUpdate(dt);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAPTURE DRAW CALLS IN FRAMEBUFFER
		{
			Renderer::ResetStats();
			
			m_Framebuffer->Bind();
			{
				RenderCommand::SetClearColour(glm::make_vec4(m_ClearColour));
				RenderCommand::Clear();

				m_Framebuffer->ClearColourAttachment(1, -1);
				
				m_ActiveScene->OnUpdateEditor(dt, m_EditorCamera);

				if (m_ViewportHovered) ReadHoveredEntity();
			}
			m_Framebuffer->Unbind();
			
		}

	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_EditorCamera.OnEvent(event);
		
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressedEvent));
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

			ImVec2 topleft = ImGui::GetWindowContentRegionMin();
			ImVec2 bottomright = ImGui::GetWindowContentRegionMax();
			ImVec2 viewportOffset = ImGui::GetWindowPos();
			m_ViewportBounds[0] = { topleft.x + viewportOffset.x, topleft.y + viewportOffset.y };
			m_ViewportBounds[1] = { bottomright.x + viewportOffset.x, bottomright.y + viewportOffset.y };

			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();

			
			if (m_ViewportFocused)
			{
				// We want to use the alt key to toggle camera controls, but ImGui uses it for
				// mouseless navigation, which will automatically defocus our viewport. To avoid
				// that happening, we give this window ownership of the alt key
				ImGui::SetKeyOwner(ImGuiMod_Alt, ImGui::GetItemID(), ImGuiInputFlags_LockThisFrame);
			}

			Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered || !m_ViewportFocused);

			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			size_t framebufferTextureID = m_Framebuffer->GetColourAttachmentID();
			ImGui::Image(reinterpret_cast<void*>(framebufferTextureID), ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2( 0, 1 ), ImVec2( 1, 0 ));

			RenderGizmos();

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

			ImGui::Text("Quads: %u", Renderer::GetStats().QuadCount);
			ImGui::Text("Draw calls: %u", Renderer::GetStats().DrawCalls);
			ImGui::Text("Hovered entity: %s", m_HoveredEntity.HasComponents<TagComponent>() ?
				m_HoveredEntity.GetComponents<TagComponent>().Tag.c_str() : "none");

			ImGui::End();
		}

	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Keyboard shortcuts
		
		if (ImGuizmo::IsUsing()) return false; // avoids crash bug when an entity is deleted during manipulation

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
			case KeyCode::Delete:
			{
				Entity selection = m_SceneHierarchyPanel.GetSelectedEntity();
				if (selection) m_ActiveScene->DestroyEntity(selection);
				break;
			}
			default:
				break;
			}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		// block other mouse input when we're trying to use imguizmo, or move our camera around
		if (ImGuizmo::IsOver() || m_EditorCamera.Controlled()) return false;

		switch (event.GetMouseButton())
		{
			case MouseCode::ButtonLeft:
			{
				if (m_ViewportHovered) m_SceneHierarchyPanel.SelectEntity(m_HoveredEntity);
				break;
			}
		}

		return true;
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

	void EditorLayer::ReadHoveredEntity()
	{
		ImVec2 mouse = ImGui::GetMousePos();
		mouse.x -= m_ViewportBounds[0].x;
		mouse.y -= m_ViewportBounds[0].y;
		mouse.y = m_ViewportSize.y - mouse.y;

		int hoveredID = m_Framebuffer->ReadPixel(1, (int)mouse.x, (int)mouse.y);

		m_HoveredEntity = (hoveredID == -1) ? Entity() : Entity((entt::entity)hoveredID, m_ActiveScene.get());

	}

	void EditorLayer::RenderGizmos()
	{
		Entity selection = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selection && m_GizmoType != -1)
		{
			// Configure ImGuizmo
			ImGuizmo::SetOrthographic(false); // TODO: make this work with orth cameras too!
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Editor camera
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			auto& tc = selection.GetComponents<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = (m_GizmoType == 1) ? 45.0f : 0.5f;
			float snapVector[3] = { snapValue, snapValue, snapValue };
			
			if (m_ViewportHovered)
			{
				// Pass data to ImGuizmo
				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType,
					ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapVector : nullptr);
				// TODO (BUG): it seems imguizmo is computing a slightly incorrect rotation value (consistently off by .81 deg), making it impossible to snap properly

				// Feedback manipulated transform
				if (ImGuizmo::IsUsing() && !m_EditorCamera.Controlled())
					Maths::DecomposeTransform(transform, tc.Translation, tc.EulerAngles, tc.Scale);
			}

		}
	}
}

