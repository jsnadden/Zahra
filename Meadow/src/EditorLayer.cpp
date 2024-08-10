#include "EditorLayer.h"

#include <ImGui/imgui_internal.h>
#include <ImGui/imgui.h>
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

		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;

		auto commandLineArgs = Application::Get().GetCommandLineArgs();
		if (commandLineArgs.Count > 1)
		{
			auto sceneFilePath = commandLineArgs[1];
			SceneSerialiser serialiser(m_EditorScene);
			serialiser.DeserialiseYaml(sceneFilePath);
		}

		m_EditorCamera = EditorCamera(.5f, 1.78f, .1f, 1000.f);

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_Icons["Play"] = Texture2D::Create("Resources/Icons/Controls/play.png");
		m_Icons["Stop"] = Texture2D::Create("Resources/Icons/Controls/stop.png");

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

		if (m_ViewportHovered && m_SceneState == SceneState::Edit)
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
				
				// UPDATE SCENE
				if (m_SceneState == SceneState::Edit)
				{
					m_ActiveScene->OnUpdateEditor(dt, m_EditorCamera);
				}
				else
				{
					m_ActiveScene->OnUpdateRuntime(dt);
				}

				if (m_ViewportHovered) ReadHoveredEntity();
			}
			m_Framebuffer->Unbind();
			
		}
	}

	void EditorLayer::OnEvent(Event& event)
	{
		if (m_ViewportHovered) m_EditorCamera.OnEvent(event);

		m_ContentBrowserPanel.OnEvent(event);
		
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressedEvent));
	}

	void EditorLayer::OnImGuiRender()
	{
		UIMenuBar();

		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton);

		UIViewport();
		UIControls();
		UIStatsWindow();

		m_SceneHierarchyPanel.OnImGuiRender(m_SceneState == SceneState::Edit);
		m_ContentBrowserPanel.OnImGuiRender();
	}

	void EditorLayer::ScenePlay()
	{
		m_SceneState = SceneState::Play;

		m_ActiveScene = Scene::CopyScene(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();
	}

	void EditorLayer::SceneStop()
	{
		m_SceneState = SceneState::Edit;

		m_ActiveScene->OnRuntimeStop();
		m_ActiveScene = m_EditorScene;
	}

	void EditorLayer::UIMenuBar()
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

			ImGui::EndMainMenuBar();
		}

	}

	void EditorLayer::UIControls()
	{
		ImGui::Begin("Controls");

		float iconSize = 35.f;

		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
		if (m_SceneState == SceneState::Edit)
		{
			if (ImGui::ImageButton((ImTextureID)m_Icons["Play"]->GetRendererID(),
				{ iconSize, iconSize }, { 0,1 }, { 1,0 }, 0))
			{
				ScenePlay();
			}
		}
		else if (m_SceneState == SceneState::Play)
		{
			if (ImGui::ImageButton((ImTextureID)m_Icons["Stop"]->GetRendererID(),
				{ iconSize, iconSize }, { 0,1 }, { 1,0 }, 0))
			{
				SceneStop();
			}
		}
		ImGui::PopStyleColor();

		ImGui::End();
	}

	void EditorLayer::UIViewport()
	{
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoCollapse);

			ImVec2 topleft = ImGui::GetWindowContentRegionMin(); // TODO: replace these obsolete functions
			ImVec2 bottomright = ImGui::GetWindowContentRegionMax();
			ImVec2 viewportOffset = ImGui::GetWindowPos();
			m_ViewportBounds[0] = { topleft.x + viewportOffset.x, topleft.y + viewportOffset.y };
			m_ViewportBounds[1] = { bottomright.x + viewportOffset.x, bottomright.y + viewportOffset.y };

			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();

			// must set false!!
			Application::Get().GetImGuiLayer()->BlockEvents(false);

			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			size_t framebufferTextureID = m_Framebuffer->GetColourAttachmentID();
			ImGui::Image(reinterpret_cast<void*>(framebufferTextureID), ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));

			if (m_SceneState == SceneState::Edit)
			{
				UIGizmos();
				ReceiveDragDrop();
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}
	}

	void EditorLayer::UIGizmos()
	{
		Entity selection = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selection && m_GizmoType != -1)
		{			
			// Configure ImGuizmo
			ImGuizmo::SetOrthographic(false); // TODO: make this work with orth cameras instead, and move to pure 2D
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);
			ImGuizmo::SetGizmoSizeClipSpace(.15);

			// Gizmo style
			{
				ImGuizmo::Style* style = &ImGuizmo::GetStyle();
				style->TranslationLineThickness = 6.0f;
				style->TranslationLineArrowSize = 12.0f;
				style->RotationLineThickness = 6.0f;
				style->RotationOuterLineThickness = 6.0f;
				style->ScaleLineThickness = 6.0f;
				style->ScaleLineCircleSize = 12.0f;
				style->CenterCircleSize = 8.0f;

				ImVec4* colors = style->Colors;
				colors[ImGuizmo::DIRECTION_X]			= ImVec4(.80f, .10f, .15f, .80f);
				colors[ImGuizmo::DIRECTION_Y]			= ImVec4(.20f, .70f, .20f, .80f);
				colors[ImGuizmo::DIRECTION_Z]			= ImVec4(.10f, .25f, .80f, .80f);
				colors[ImGuizmo::PLANE_X]				= ImVec4(.80f, .10f, .15f, .80f);
				colors[ImGuizmo::PLANE_Y]				= ImVec4(.20f, .70f, .20f, .80f);
				colors[ImGuizmo::PLANE_Z]				= ImVec4(.10f, .25f, .80f, .80f);
				colors[ImGuizmo::SELECTION]				= ImVec4(.97f, .77f, .22f, .80f);
				colors[ImGuizmo::ROTATION_USING_BORDER]	= ImVec4(.97f, .77f, .22f, .80f);
				colors[ImGuizmo::ROTATION_USING_FILL]	= ImVec4(.97f, .77f, .22f, .80f);
				colors[ImGuizmo::TEXT]					= ImVec4(.98f, .95f, .89f, .99f);
				colors[ImGuizmo::TEXT_SHADOW]			= ImVec4(.10f, .10f, .10f, .99f);
				colors[ImGuizmo::HATCHED_AXIS_LINES]	= ImVec4(.00f, .00f, .00f, .00f);
			}

			// Editor camera
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			auto& tc = selection.GetComponents<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = (m_GizmoType == 120) ? 45.0f : 0.5f;
			float snapVector[3] = { snapValue, snapValue, snapValue };

			// Pass data to ImGuizmo
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType,
				ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapVector : nullptr);
			
			// Feedback manipulated transform
			if (ImGuizmo::IsUsing() && !m_EditorCamera.Controlled())
				Maths::DecomposeTransform(transform, tc.Translation, tc.EulerAngles, tc.Scale);

		}
	}

	void EditorLayer::UIStatsWindow()
	{
		ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoCollapse);

		ImGui::Text("Quads: %u", Renderer::GetStats().QuadCount);
		ImGui::Text("Draw calls: %u", Renderer::GetStats().DrawCalls);
		ImGui::TextWrapped("Hovered entity: %s", m_HoveredEntity.HasComponents<TagComponent>() ?
			m_HoveredEntity.GetComponents<TagComponent>().Tag.c_str() : "none");

		ImGui::End();
	}

	void EditorLayer::ReceiveDragDrop()
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BROWSER_FILE_SCENE"))
			{
				char filepath[256];
				strcpy_s(filepath, (const char*)payload->Data);
				OpenSceneFile(filepath);
			}

			ImGui::EndDragDropTarget();
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
				if (m_ViewportFocused) m_GizmoType = -1;
				break;
			}
			case KeyCode::W:
			{
				if (m_ViewportFocused) m_GizmoType = 7;
				break;
			}
			case KeyCode::E:
			{
				if (m_ViewportFocused) m_GizmoType = 120;
				break;
			}
			case KeyCode::R:
			{
				if (m_ViewportFocused) m_GizmoType = 896;
				break;
			}
			case KeyCode::Delete:
			{
				Entity& selection = m_SceneHierarchyPanel.GetSelectedEntity();
				if (selection)
				{
					m_ActiveScene->DestroyEntity(selection); // TODO: avoid deleting certain objects (e.g. those subject to Box2D)
					m_SceneHierarchyPanel.SelectEntity({});
				}
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
		if (m_EditorCamera.Controlled() || (ImGuizmo::IsOver() && m_SceneHierarchyPanel.GetSelectedEntity())) return false;
		
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
		if (m_SceneState != SceneState::Edit) SceneStop();

		{
			m_EditorScene = CreateRef<Scene>();

			m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
			m_CurrentFilePath.clear();

			m_ActiveScene = m_EditorScene;
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		}
	}

	void EditorLayer::OpenSceneFile()
	{
		std::filesystem::path filepath = FileDialogs::OpenFile(m_FileTypesFilter[0], m_FileTypesFilter[1]);
		OpenSceneFile(filepath);
	}

	void EditorLayer::OpenSceneFile(std::filesystem::path filepath)
	{
		if (filepath.empty()) return;
		
		if (m_SceneState != SceneState::Edit) SceneStop();

		if (filepath.extension().string() != ".zsc")
		{
			Z_WARN("Couldn't open {0} - scene files must have extension '.zsc'", filepath.filename().string());
			return;
		}

		Ref<Scene> newScene = CreateRef<Scene>();
		SceneSerialiser serialiser(newScene);
		if (serialiser.DeserialiseYaml(filepath.string()))
		{
			m_EditorScene = newScene;

			m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);

			std::string sceneName = filepath.filename().string();
			m_EditorScene->SetName(sceneName);
			m_CurrentFilePath = filepath;

			m_ActiveScene = m_EditorScene;
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		}

		// TODO: report/display success of file open
	}

	void EditorLayer::SaveSceneFile()
	{
		if (m_CurrentFilePath.empty())
		{
			SaveAsSceneFile();
		}
		else
		{
			std::string sceneName = m_CurrentFilePath.filename().string();
			m_EditorScene->SetName(sceneName);
			
			SceneSerialiser serialiser(m_EditorScene);
			serialiser.SerialiseYaml(m_CurrentFilePath.string());
		}

		// TODO: report/display success of file save
	}

	void EditorLayer::SaveAsSceneFile()
	{
		std::filesystem::path filepath = FileDialogs::SaveFile(m_FileTypesFilter[0], m_FileTypesFilter[1]);
		if (!filepath.empty())
		{
			std::string sceneName = filepath.filename().string();
			m_EditorScene->SetName(sceneName);
			m_CurrentFilePath = filepath;

			SceneSerialiser serialiser(m_EditorScene);
			serialiser.SerialiseYaml(m_CurrentFilePath.string());
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

	
}

