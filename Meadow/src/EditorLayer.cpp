#include "EditorLayer.h"

#include "Zahra/Scene/SceneSerialiser.h"
#include "Zahra/Utils/PlatformUtils.h"
#include "Zahra/Maths/Maths.h"

#include <ImGui/imgui_internal.h>
#include <ImGui/imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Zahra
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer") {}

	void EditorLayer::OnAttach()
	{
		auto imguiLayer = ImGuiLayer::GetOrCreate();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// VIEWPORT FRAMEBUFFER
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.Name = "Editor_ViewportFramebuffer";
			framebufferSpec.Width = 1;
			framebufferSpec.Height = 1;
			framebufferSpec.ClearColour = { .0f, .0f, .0f };
			{
				auto& attachment = framebufferSpec.ColourAttachmentSpecs.emplace_back();
				attachment.Format = ImageFormat::SRGBA;
			}
			framebufferSpec.HasDepthStencil = true;
			framebufferSpec.DepthClearValue = 1.0f;
			framebufferSpec.DepthStencilAttachmentSpec.Format = ImageFormat::DepthStencil;
			m_ViewportFramebuffer = Framebuffer::Create(framebufferSpec);

			m_ViewportTexture = Texture2D::CreateFromImage2D(m_ViewportFramebuffer->GetColourAttachment(0));
			m_ViewportTextureHandle = imguiLayer->RegisterTexture(m_ViewportTexture);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEMPORARY
		{
			Zahra::RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "Editor_ClearPass";
			renderPassSpec.RenderTarget = m_ViewportFramebuffer;
			renderPassSpec.ClearColourAttachments = true;
			renderPassSpec.ClearDepthAttachment = true;
			m_ClearPass = Zahra::RenderPass::Create(renderPassSpec);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SCENE SYSTEM
		{
			m_EditorScene = Ref<Scene>::Create();
			m_ActiveScene = m_EditorScene;

			Renderer2DSpecification renderer2DSpec{};
			renderer2DSpec.RenderTarget = m_ViewportFramebuffer;
			m_Renderer2D = Ref<Renderer2D>::Create(renderer2DSpec);
			m_Renderer2D->SetLineWidth(3.f);

			auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
			if (commandLineArgs.Count > 1)
			{
				auto sceneFilePath = commandLineArgs[1];
				SceneSerialiser serialiser(m_EditorScene);
				serialiser.DeserialiseYaml(sceneFilePath);
			}

			m_SceneHierarchyPanel.SetContext(m_ActiveScene);
			m_SceneHierarchyPanel.SetEditorCamera(m_EditorCamera);
		}

		Texture2DSpecification textureSpec{};
		m_Icons["Play"]		= Texture2D::CreateFromFile(textureSpec, "Resources/Icons/Controls/play.png");
		m_Icons["Stop"]		= Texture2D::CreateFromFile(textureSpec, "Resources/Icons/Controls/stop.png");
		m_Icons["PlaySim"]	= Texture2D::CreateFromFile(textureSpec, "Resources/Icons/Controls/play_sim.png");
		m_Icons["Replay"]	= Texture2D::CreateFromFile(textureSpec, "Resources/Icons/Controls/replay.png");

		for (auto& [name, texture] : m_Icons)
		{
			m_IconHandles[name] = imguiLayer->RegisterTexture(texture);
		}

	}

	void EditorLayer::OnDetach()
	{
		m_Renderer2D.Reset();
		m_ActiveScene.Reset();
		m_EditorScene.Reset();

		m_ClearPass.Reset();

		ImGuiLayer::GetOrCreate()->DeregisterTexture(m_ViewportTextureHandle);
		m_ViewportTextureHandle = nullptr;
		m_ViewportTexture.Reset();
		m_ViewportFramebuffer.Reset();
	}

	void EditorLayer::OnUpdate(float dt)
	{
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// UPDATE VIEWPORT AND CAMERAS
		{
			if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && (m_ViewportFramebuffer->GetWidth() != m_ViewportSize.x || m_ViewportFramebuffer->GetHeight() != m_ViewportSize.y))
			{
				ImGuiLayer::GetOrCreate()->DeregisterTexture(m_ViewportTextureHandle);

				m_ViewportFramebuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
				m_ViewportTexture->Resize(m_ViewportSize.x, m_ViewportSize.y);
				m_ViewportTextureHandle = ImGuiLayer::GetOrCreate()->RegisterTexture(m_ViewportTexture);

				m_ClearPass->OnResize();

				m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
				m_Renderer2D->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);

				m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			}

			if (m_ViewportHovered && m_SceneState != SceneState::Play)
			{
				m_EditorCamera.OnUpdate(dt);
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// UPDATE AND RENDER ACTIVE SCENE
		{
			Zahra::Renderer::BeginRenderPass(m_ClearPass, false, true);
			Zahra::Renderer::EndRenderPass();

			//m_Scene->OnRenderEditor(m_Renderer2D, m_Camera);

			// TODO: mousepicking
			//m_Framebuffer->ClearColourAttachment(1, -1);

			switch (m_SceneState)
			{
				case SceneState::Edit:
				{
					m_ActiveScene->OnUpdateEditor(dt);
					m_ActiveScene->OnRenderEditor(m_Renderer2D, m_EditorCamera);
					break;
				}
				case SceneState::Play:
				{
					// TODO: ressurect
					//m_ActiveScene->OnUpdateRuntime(dt);
					break;
				}
				case SceneState::Simulate:
				{
					// TODO: ressurect
					// m_ActiveScene->OnUpdateSimulation(dt, m_EditorCamera);
					break;
				}

				default:
				{
					Z_CORE_ASSERT(false, "Invalid SceneState");
				}
			}

			UIHighlightSelection();

			if (m_ViewportHovered)
			{
				ReadHoveredEntity();
			}
			
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		UIMenuBar();
		UIAboutWindow();

		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton);

		UIViewport();
		UIControls();
		UIStatsWindow();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
	}

	void EditorLayer::ScenePlay()
	{
		m_HoveredEntity = {};

		m_SceneState = SceneState::Play;

		m_ActiveScene = Scene::CopyScene(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::SceneSimulate()
	{
		m_HoveredEntity = {};

		m_SceneState = SceneState::Simulate;

		m_ActiveScene = Scene::CopyScene(m_EditorScene);
		m_ActiveScene->OnSimulationStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::SceneStop()
	{
		m_HoveredEntity = {};

		switch (m_SceneState)
		{
			case SceneState::Simulate:
			{
				m_ActiveScene->OnSimulationStop();
				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnRuntimeStop();
				break;
			}
			default:
				Z_CORE_ASSERT(false, "SceneStop should only be called when SceneState is ::Simulate or ::Play");
		}

		m_SceneState = SceneState::Edit;
		m_ActiveScene = m_EditorScene;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
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

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Toggle Fullscreen", "F11"))
				{
					Window& window = Application::Get().GetWindow();
					window.SetFullscreen(!window.IsFullscreen());
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

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About"))
					m_ShowAboutWindow = true;

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void EditorLayer::UIAboutWindow()
	{
		if (m_ShowAboutWindow) ImGui::OpenPopup("Meadow##About");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Meadow##About", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("The official editor for the Zahra engine.");
			
			if (ImGui::Button("Close", ImVec2(120, 0)))
			{
				m_ShowAboutWindow = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void EditorLayer::UIControls()
	{
		ImGui::Begin("Controls");
		
		float iconSize = 35.f;

		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
		switch (m_SceneState)
		{
		case SceneState::Edit:
		{
			if (ImGui::ImageButton(m_IconHandles["Play"], { iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				ScenePlay();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Play Scene");
				ImGui::EndTooltip();
			}

			ImGui::SameLine();

			if (ImGui::ImageButton(m_IconHandles["PlaySim"],
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				SceneSimulate();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Simulate Physics");
				ImGui::EndTooltip();
			}

			break;
		}
		case SceneState::Play:
		{
			if (ImGui::ImageButton(m_IconHandles["Stop"],
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				SceneStop();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Stop Scene");
				ImGui::EndTooltip();
			}

			ImGui::SameLine();

			if (ImGui::ImageButton(m_IconHandles["Replay"],
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				SceneStop();
				ScenePlay();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Restart Scene");
				ImGui::EndTooltip();
			}

			break;
		}
		case SceneState::Simulate:
		{
			if (ImGui::ImageButton(m_IconHandles["Stop"],
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				SceneStop();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Stop Simulation");
				ImGui::EndTooltip();
			}

			ImGui::SameLine();

			if (ImGui::ImageButton(m_IconHandles["Replay"],
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				SceneStop();
				SceneSimulate();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Reset Simulation");
				ImGui::EndTooltip();
			}

			break;
		}
		default:
			break;
		}
		ImGui::PopStyleColor();

		ImGui::Separator();
		ImGui::Text("Debug overlay:");

		auto& sceneDebugSettings = m_ActiveScene->GetDebugRenderSettings();
		ImGui::Checkbox("Show colliders", &sceneDebugSettings.ShowColliders);
		ImGui::ColorEdit4("Collider colour", glm::value_ptr(sceneDebugSettings.ColliderColour), ImGuiColorEditFlags_NoInputs);
		//m_ActiveScene->SetOverlayMode(overlayMode);

		ImGui::Separator();
		ImGui::Text("Editor");

		ImGui::ColorEdit4("Selection colour", glm::value_ptr(m_HighlightSelectionColour), ImGuiColorEditFlags_NoInputs);

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
			ImGuiLayer::GetOrCreate()->BlockEvents(false);

			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			ImGuiTextureHandle viewportTextureID = m_ViewportTextureHandle;
			ImGui::Image(viewportTextureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 0), ImVec2(1, 1));

			if (m_SceneState == SceneState::Edit) UIGizmos();

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
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);
			ImGuizmo::SetGizmoSizeClipSpace(.15f);

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
			glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
			cameraProjection[1][1] *= -1.f;
			glm::mat4 cameraView = m_EditorCamera.GetView();

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

	// TODO: move highight rendering to Scene, or SceneRenderer
	void EditorLayer::UIHighlightSelection()
	{
		/*if (Entity selection = m_SceneHierarchyPanel.GetSelectedEntity())
		{
			TransformComponent entityTransform = selection.GetComponents<TransformComponent>();

			if (m_SceneState == SceneState::Play)
			{
				Entity cameraEntity = m_ActiveScene->GetActiveCamera();

				if (cameraEntity)
				{
					Camera camera = cameraEntity.GetComponents<CameraComponent>().Camera;
					glm::mat4 cameraTransform = cameraEntity.GetComponents<TransformComponent>().GetTransform();
					m_Renderer2D->BeginScene(camera, cameraTransform);
					m_Renderer2D->DrawQuadBoundingBox(entityTransform.GetTransform(), m_HighlightSelectionColour);
					m_Renderer2D->EndScene();
				}
			}
			else
			{
				m_Renderer2D->BeginScene(m_EditorCamera);
				m_Renderer2D->DrawQuadBoundingBox(entityTransform.GetTransform(), m_HighlightSelectionColour);
				m_Renderer2D->EndScene();
			}
			
			
		}*/
	}

	void EditorLayer::UIStatsWindow()
	{
		auto stats = m_Renderer2D->GetStats();

		if (ImGui::Begin("Renderer Stats", NULL, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Text("Quads: %u", stats.QuadCount);
			ImGui::Text("Circles: %u", stats.CircleCount);
			ImGui::Text("Lines: %u", stats.LineCount);
			ImGui::Text("Draw calls: %u", stats.DrawCalls);
			ImGui::TextWrapped("Hovered entity: %s", m_HoveredEntity.HasComponents<TagComponent>() ?
				m_HoveredEntity.GetComponents<TagComponent>().Tag.c_str() : "none");

			ImGui::End();
		}		
	}

	void EditorLayer::OnEvent(Event& event)
	{
		if (m_ViewportHovered)
			m_EditorCamera.OnEvent(event);

		m_ContentBrowserPanel.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressedEvent));
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Keyboard shortcuts
		
		if (m_SceneState == SceneState::Edit)
		{
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
				case KeyCode::D:
				{
					if (ctrl && m_SceneState == SceneState::Edit)
					{
						Entity& selection = m_SceneHierarchyPanel.GetSelectedEntity();
						if (selection)
						{
							Entity copy = m_EditorScene->DuplicateEntity(selection);
							m_SceneHierarchyPanel.SelectEntity(copy);
						}
						return true;
					}

					break;
				}
				case KeyCode::F11:
				{
					Window& window = Application::Get().GetWindow();
					window.SetFullscreen(!window.IsFullscreen());
					return true;
				}
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		// block other mouse input when we're trying to use imguizmo, or move our camera around
		if (m_EditorCamera.Controlled() || ( ImGuizmo::IsOver() && m_SceneHierarchyPanel.GetSelectedEntity() ))
			return false;
		
		if (event.GetMouseButton() == MouseCode::ButtonLeft)
		{
			if (m_ViewportHovered)
				m_SceneHierarchyPanel.SelectEntity(m_HoveredEntity);
		}

		return true;
	}

	void EditorLayer::NewScene()
	{
		if (m_SceneState != SceneState::Edit) SceneStop();

		{
			m_EditorScene = Ref<Scene>::Create();

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

		m_HoveredEntity = {};

		if (filepath.extension().string() != ".zsc")
		{
			Z_WARN("Couldn't open {0} - scene files must have extension '.zsc'", filepath.filename().string());
			return;
		}

		Ref<Scene> newScene = Ref<Scene>::Create();
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
		// TODO: Figure out how to make mousepicking work in Vulkan
		// Might have to use ray casting
		/*ImVec2 mouse = ImGui::GetMousePos();
		mouse.x -= m_ViewportBounds[0].x;
		mouse.y -= m_ViewportBounds[0].y;
		mouse.y = m_ViewportSize.y - mouse.y;

		int hoveredID = m_Framebuffer->ReadPixel(1, (int)mouse.x, (int)mouse.y);

		m_HoveredEntity = (hoveredID == -1) ? Entity() : Entity((entt::entity)hoveredID, m_ActiveScene.Raw());*/
	}

	
}

