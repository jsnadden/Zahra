#include "EditorLayer.h"

#include "Editor/Editor.h"

#include "Zahra/Maths/Maths.h"
#include "Zahra/Projects/Project.h"
#include "Zahra/Scene/SceneSerialiser.h"
#include "Zahra/Utils/PlatformUtils.h"

#include <glm/gtc/type_ptr.hpp>
#include <ImGui/imgui_internal.h>
#include <ImGui/imgui.h>
#include <ImGuizmo.h>
#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

namespace Zahra
{
	static FileTypeFilter s_DLLFilter = { "Dynamic Link Library", "*.dll" };
	static FileTypeFilter s_ProjectFilter = { "Zahra Project", "*.zpj" };
	static FileTypeFilter s_SceneFilter = { "Zahra Scene", "*.zsc" };

	// TODO: define some global preprocessor constants for max project
	// name/filepath lengths, instead of choosing ad hoc values here
	size_t s_NewProjectNameBufferLength = 64;
	size_t s_NewProjectLocationBufferLength = 128;
	static char* s_NewProjectNameBuffer = znew char[s_NewProjectNameBufferLength];
	static char* s_NewProjectLocationBuffer = znew char[s_NewProjectLocationBufferLength];

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		memset(s_NewProjectNameBuffer, 0, s_NewProjectNameBufferLength);
		memset(s_NewProjectLocationBuffer, 0, s_NewProjectLocationBufferLength);
	}

	EditorLayer::~EditorLayer()
	{
		zdelete[] s_NewProjectNameBuffer;
		zdelete[] s_NewProjectLocationBuffer;
	}

	void EditorLayer::OnAttach()
	{
		LoadConfigFile();
		Editor::SetPrimaryEditorCamera(m_EditorCamera);
		m_AutosaveEnabled = false;

		EditorIcons::Init();

		m_SceneHierarchyPanel.CacheScriptClassNames();

		// Open project/scene
		{
			m_EditorScene = Ref<Scene>::Create();
			m_ActiveScene = m_EditorScene;
			Editor::SetSceneContext(m_ActiveScene);

			m_HaveActiveProject = Project::Load(m_WorkingProjectFilepath);
			
			if (m_HaveActiveProject)
			{
				m_ContentBrowserPanel.OnLoadProject();

				TryLoadProjectScriptAssembly(Project::GetScriptAssemblyFilepath());

				if (m_WorkingSceneRelativePath.empty())
				{
					auto startingScene = Project::GetStartingSceneFilepath();
					if (startingScene.empty())
						NewScene();
					else
						OpenSceneFile(startingScene);
				}
				else
				{
					auto workingScenePath = Project::GetProjectDirectory() / m_WorkingSceneRelativePath;
					if (!OpenSceneFile(workingScenePath))
						NewScene();
				}
			}
			else
			{
				m_WorkingProjectFilepath.clear();
				m_WorkingSceneRelativePath.clear();
				m_ShowNewProjectWindow = true;
			}			
		}

		// Setup viewport
		{
			Image2DSpecification imageSpec{};
			imageSpec.Name = "Editor_ColourPickingAttachment";
			imageSpec.Format = ImageFormat::R32_SI;
			imageSpec.Width = 1;
			imageSpec.Height = 1;
			imageSpec.Sampled = true;
			imageSpec.TransferSource = true;
			imageSpec.CreatePixelBuffer = true;
			m_ColourPickingAttachment = Image2D::Create(imageSpec);

			FramebufferSpecification framebufferSpec{};
			framebufferSpec.Name = "Editor_ViewportFramebuffer";
			framebufferSpec.Width = 1;
			framebufferSpec.Height = 1;
			{
				auto& attachment = framebufferSpec.ColourAttachmentSpecs.emplace_back();
				attachment.Format = ImageFormat::RGBA_UN;
			}
			{
				auto& attachment = framebufferSpec.ColourAttachmentSpecs.emplace_back();
				attachment.InheritFrom = m_ColourPickingAttachment;
				attachment.Format = ImageFormat::R32_SI;
				attachment.ClearColour.iColour = glm::ivec4(-1, 0, 0, 1);
			}
			framebufferSpec.HasDepthStencil = true;
			framebufferSpec.DepthClearValue = 1.0f;
			framebufferSpec.DepthStencilAttachmentSpec.Format = ImageFormat::DepthStencil;
			m_ViewportFramebuffer = Framebuffer::Create(framebufferSpec);

			m_ViewportTexture = Texture2D::CreateFromImage2D(m_ViewportFramebuffer->GetColourAttachment(0));
			m_ViewportTextureHandle = ImGuiLayer::GetOrCreate()->RegisterTexture(m_ViewportTexture);
		}

		// TODO: this stuff should really be in SceneRenderer
		{
			Zahra::RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "Editor_ClearPass";
			renderPassSpec.RenderTarget = m_ViewportFramebuffer;
			renderPassSpec.ClearColourAttachments = true;
			renderPassSpec.ClearDepthAttachment = true;
			renderPassSpec.ManagesResources = false;
			m_ClearPass = Zahra::RenderPass::Create(renderPassSpec);

			Renderer2DSpecification renderer2DSpec{};
			renderer2DSpec.RenderTarget = m_ViewportFramebuffer;
			m_Renderer2D = Ref<Renderer2D>::Create(renderer2DSpec);
			m_Renderer2D->SetLineWidth(3.f);
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

		EditorIcons::Shutdown();
	}

	void EditorLayer::OnUpdate(float dt)
	{
		if (m_FramerateRefreshTimer.Elapsed() >= c_FramerateRefreshInterval)
		{
			m_FramerateRefreshTimer.Reset();
			m_Framerate = 1.0f / dt;
		}

		if (m_AutosaveTimer.Elapsed() >= Editor::GetConfig().AutosaveInterval)
		{
			m_AutosaveTimer.Reset();

			if (m_AutosaveEnabled)
			{
				// TODO: save project? editor config?
				SaveSceneFile();
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// UPDATE VIEWPORT AND CAMERAS
		{
			if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && (m_ViewportFramebuffer->GetWidth() != m_ViewportSize.x || m_ViewportFramebuffer->GetHeight() != m_ViewportSize.y))
			{
				ImGuiLayer::GetOrCreate()->DeregisterTexture(m_ViewportTextureHandle);

				m_ColourPickingAttachment->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				m_ViewportFramebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				m_ViewportTexture->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				m_ViewportTextureHandle = ImGuiLayer::GetOrCreate()->RegisterTexture(m_ViewportTexture);

				m_ClearPass->OnResize();

				m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
				m_Renderer2D->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

				m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			}

			if (m_ViewportHovered && (Editor::GetSceneState() == SceneState::Edit || Editor::GetSceneState() == SceneState::Simulate))
			{
				m_EditorCamera.OnUpdate(dt);
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// UPDATE AND RENDER ACTIVE SCENE
		{
			Zahra::Renderer::BeginRenderPass(m_ClearPass, false, true);
			Zahra::Renderer::EndRenderPass();

			switch (Editor::GetSceneState())
			{
				case SceneState::Edit:
				{
					m_ActiveScene->OnUpdateEditor(dt);
					m_ActiveScene->OnRenderEditor(m_Renderer2D, m_EditorCamera, Editor::GetSelectedEntity(), m_HighlightSelectionColour);
					break;
				}
				case SceneState::Play:
				{
					if (!m_Paused || m_StepCountdown > 0)
					{
						m_ActiveScene->OnUpdateRuntime(dt);

						if (m_StepCountdown > 0)
							m_StepCountdown--;
					}

					m_ActiveScene->OnRenderRuntime(m_Renderer2D, Editor::GetSelectedEntity(), m_HighlightSelectionColour);
					break;
				}
				case SceneState::Simulate:
				{
					if (!m_Paused || m_StepCountdown > 0)
					{
						m_ActiveScene->OnUpdateSimulation(dt);

						if (m_StepCountdown > 0)
							m_StepCountdown--;
					}
					
					m_ActiveScene->OnRenderEditor(m_Renderer2D, m_EditorCamera, Editor::GetSelectedEntity(), m_HighlightSelectionColour);
					break;
				}

				default:
				{
					Z_CORE_ASSERT(false, "Invalid SceneState");
				}
			}

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
		UISceneControls();
		UIStatsWindow();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();

		UINewProjectWindow();
		UISaveChangesPrompt();
	}

	void EditorLayer::ScenePlay()
	{
		m_HoveredEntity = {};

		/*if (Editor::GetSceneState() == SceneState::Simulate)
			m_ActiveScene->OnSimulationStop();*/

		Editor::SetSceneState(SceneState::Play);

		m_ActiveScene = Scene::CopyScene(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		Editor::SetSceneContext(m_ActiveScene);
	}

	void EditorLayer::SceneSimulate()
	{
		m_HoveredEntity = {};

		Editor::SetSceneState(SceneState::Simulate);

		m_ActiveScene = Scene::CopyScene(m_EditorScene);
		m_ActiveScene->OnSimulationStart();

		Editor::SetSceneContext(m_ActiveScene);
	}

	void EditorLayer::SceneStop()
	{
		m_HoveredEntity = {};

		switch (Editor::GetSceneState())
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
				break;
		}

		Editor::SetSceneState(SceneState::Edit);
		m_ActiveScene = m_EditorScene;

		Editor::SetSceneContext(m_ActiveScene);
	}

	void EditorLayer::UIMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{			

			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Project..."))
					m_ShowNewProjectWindow = true;

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
					OpenSceneFile();

				ImGui::Separator();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))					
					SaveSceneFile();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneFileAs();

				ImGui::Separator();

				if (ImGui::MenuItem("Exit", "Alt+F4"))
				{
					DoAfterHandlingUnsavedChanges([this]()
						{
							if (m_HaveActiveProject)
								SaveProjectFile();

							SaveEditorConfigFile();

							Application::Get().Exit();
						},
						"Save scene before exiting?", true);
				}

				ImGui::EndMenu();
			}

			/*if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z", false, Editor::CanUndo()))
				{
					Editor::Undo();
				}

				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, Editor::CanRedo()))
				{
					Editor::Redo();
				}

				ImGui::EndMenu();
			}*/

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
				ImGui::SeparatorText("Transform Gizmo");

				if (ImGui::MenuItem("Select", "Q"))
					m_GizmoType = TransformationType::None;

				if (ImGui::MenuItem("Translate", "W"))
					m_GizmoType = TransformationType::Translation;

				if (ImGui::MenuItem("Rotate", "E"))
					m_GizmoType = TransformationType::Rotation;

				if (ImGui::MenuItem("Scale", "R"))
					m_GizmoType = TransformationType::Scale;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Scripts"))
			{
				bool canLoadAssembly = Editor::GetSceneState() == SceneState::Edit && m_HaveActiveProject && !ScriptEngine::AppAssemblyAlreadyLoaded();
				if (ImGui::MenuItem("Load Project Assembly...", "", false, canLoadAssembly))
				{
					std::filesystem::path filepath = FileDialogs::OpenFile(s_DLLFilter);
					if (TryLoadProjectScriptAssembly(filepath))
					{
						// TODO: might be necessary if we want to swap out assemblies
						/*DoAfterHandlingUnsavedChanges([this]()
							{
								if (m_WorkingSceneRelativePath.empty())
									NewScene();
								else
									OpenSceneFile(Project::GetProjectDirectory() / m_WorkingSceneRelativePath);
							},
							"Need to reload script data.\nSave changes to current scene?", false);*/
					}
				}
				
				bool canReloadAssembly = Editor::GetSceneState() == SceneState::Edit && ScriptEngine::AppAssemblyAlreadyLoaded();
				if (ImGui::MenuItem("Reload Assembly", "Ctrl+Shift+R", false, canReloadAssembly))
				{
					ScriptEngine::ReloadAssembly();
				}

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
		if (m_ShowAboutWindow)
			ImGui::OpenPopup("Meadow##About");

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

	void EditorLayer::UISceneControls()
	{
		auto sceneState = Editor::GetSceneState();
		bool edit = sceneState == SceneState::Edit;
		bool play = sceneState == SceneState::Play;
		bool simulate = sceneState == SceneState::Simulate;
		float iconSize = 35.f;

		ImGui::Begin("Controls");
		
		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });

		// Play/Pause buttons
		if (edit || m_Paused)
		{
			if (ImGui::ImageButton(EditorIcons::GetIconHandle("SceneControls/Play"), { iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				if (edit)
					ScenePlay();
				else
					m_Paused = false;
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				if (edit)
					ImGui::Text("Begin Runtime");
				else
					ImGui::Text("Resume");
				ImGui::EndTooltip();
			}
		}
		else
		{
			if (ImGui::ImageButton(EditorIcons::GetIconHandle("SceneControls/Pause"),
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				m_Paused = true;
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Pause");
				ImGui::EndTooltip();
			}
		}

		ImGui::SameLine();

		// PhysicsOn/Off and Stop buttons
		if (edit)
		{
			if (ImGui::ImageButton(EditorIcons::GetIconHandle("SceneControls/PhysicsOn"),
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				SceneSimulate();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Physics On");
				ImGui::EndTooltip();
			}
		}
		else if (simulate)
		{
			if (ImGui::ImageButton(EditorIcons::GetIconHandle("SceneControls/PhysicsOff"),
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				m_Paused = false;
				SceneStop();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Physics Off");
				ImGui::EndTooltip();
			}
		}
		else
		{
			if (ImGui::ImageButton(EditorIcons::GetIconHandle("SceneControls/Stop"),
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				m_Paused = false;
				SceneStop();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("End Runtime");
				ImGui::EndTooltip();
			}
		}

		// Reset button
		if (!edit)
		{
			ImGui::SameLine();

			if (ImGui::ImageButton(EditorIcons::GetIconHandle("SceneControls/Reset"),
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				if (play)
				{
					SceneStop();
					ScenePlay();
				}
				else
				{
					SceneStop();
					SceneSimulate();
				}
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Reset Scene");
				ImGui::EndTooltip();
			}
		}

		// Step button
		if (!edit && m_Paused)
		{
			ImGui::SameLine();

			if (ImGui::ImageButton(EditorIcons::GetIconHandle("SceneControls/Step"),
				{ iconSize, iconSize }, { 0, 0 }, { 1, 1 }, 0))
			{
				m_StepCountdown = m_FramesPerStep;
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::Text("Step Forward");
				ImGui::EndTooltip();
			}
		}
		ImGui::PopStyleColor();

		ImGui::Text("Frames Per Step:");
		ImGui::InputInt("##FramesPerStep", &m_FramesPerStep, 1, 2);
		if (m_FramesPerStep < 1)
			m_FramesPerStep = 1;

		ImGui::Separator();
		ImGui::Text("Debug overlay:");

		auto& sceneDebugSettings = m_ActiveScene->GetDebugRenderSettings();
		ImGui::Checkbox("Show colliders", &sceneDebugSettings.ShowColliders);
		ImGui::ColorEdit4("Collider colour", glm::value_ptr(sceneDebugSettings.ColliderColour), ImGuiColorEditFlags_NoInputs);
		//m_ActiveScene->SetOverlayMode(overlayMode);

		ImGui::Separator();
		ImGui::Text("Editor");

		ImGui::ColorEdit4("Selection colour", glm::value_ptr(m_HighlightSelectionColour), ImGuiColorEditFlags_NoInputs);
		//ImGui::DragFloat("Selection box expansion", &sceneDebugSettings.SelectionPushOut, .001f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic& ImGuiSliderFlags_NoRoundToFormat);

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

			if (Editor::GetSceneState() == SceneState::Edit)
				UIGizmo();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BROWSER_FILE_SCENE"))
				{
					char filepath[256];
					strcpy_s(filepath, (const char*)payload->Data);

					DoAfterHandlingUnsavedChanges([this, filepath]()
						{
							if (m_HaveActiveProject)
								SaveProjectFile();

							OpenSceneFile(filepath);
						},
						"Save current scene?", true);
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}
	}

	void EditorLayer::UIGizmo()
	{
		Entity selection = Editor::GetSelectedEntity();
		if (!selection || m_GizmoType == TransformationType::None || m_EditorCamera.Controlled())
			return;
		
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);
		ImGuizmo::SetGizmoSizeClipSpace(.15f);

		static bool imGuizmoStyleSet = false;
		if (!imGuizmoStyleSet)
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
			colors[ImGuizmo::DIRECTION_X]			= ImVec4(MEADOW_RED_1,		0.80f);
			colors[ImGuizmo::DIRECTION_Y]			= ImVec4(MEADOW_BLUE_1,		0.80f);
			colors[ImGuizmo::DIRECTION_Z]			= ImVec4(MEADOW_GREEN_1,	0.80f);
			colors[ImGuizmo::PLANE_X]				= ImVec4(MEADOW_RED_1,		0.80f);
			colors[ImGuizmo::PLANE_Y]				= ImVec4(MEADOW_BLUE_1,		0.80f);
			colors[ImGuizmo::PLANE_Z]				= ImVec4(MEADOW_GREEN_1,	0.80f);
			colors[ImGuizmo::SELECTION]				= ImVec4(MEADOW_YELLOW_2,	0.80f);
			colors[ImGuizmo::ROTATION_USING_BORDER]	= ImVec4(MEADOW_YELLOW_2,	0.80f);
			colors[ImGuizmo::ROTATION_USING_FILL]	= ImVec4(MEADOW_YELLOW_2,	0.80f);
			colors[ImGuizmo::TEXT]					= ImVec4(MEADOW_WHITE_1,	0.99f);
			colors[ImGuizmo::TEXT_SHADOW]			= ImVec4(MEADOW_GREY_1,		0.99f);
			colors[ImGuizmo::HATCHED_AXIS_LINES]	= ImVec4(.00f, .00f, .00f,	0.00f);

			imGuizmoStyleSet = true;
		}

		glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
		cameraProjection[1][1] *= -1.f;
		glm::mat4 cameraView = m_EditorCamera.GetView();

		auto& transformComponent = selection.GetComponents<TransformComponent>();
		glm::mat4 transform = transformComponent.GetTransform();

		bool snap = Input::IsKeyPressed(Key::LeftControl);
		float snapValue = (m_GizmoType == TransformationType::Rotation) ? 45.0f : 0.5f;
		float snapVector[3] = { snapValue, snapValue, snapValue };

		ImGuizmo::OPERATION op;
		switch (m_GizmoType)
		{
			case TransformationType::Translation:	op = ImGuizmo::TRANSLATE;	break;
			case TransformationType::Rotation:		op = ImGuizmo::ROTATE;		break;
			case TransformationType::Scale:			op = ImGuizmo::SCALE;		break;
		}

		ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), op,
			ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapVector : nullptr);

		if (ImGuizmo::IsUsing())
		{
			glm::vec3 eulers;
			Maths::DecomposeTransform(transform, transformComponent.Translation, eulers, transformComponent.Scale);
			transformComponent.SetRotation(eulers);
		}
	}

	void EditorLayer::UIStatsWindow()
	{
		auto& renderer2DStats = m_Renderer2D->GetStats();
		auto& allocationStats = Zahra::Memory::GetAllocationStats();
		auto& allocationStatsMap = Zahra::Allocator::GetAllocationStatsMap();

		if (ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::SeparatorText("Timing");
			{
				ImGui::Text("Framerate: %.2f fps", m_Framerate);
			}

			ImGui::SeparatorText("Renderer 2D");
			{
				ImGui::Text("Quads: %u", renderer2DStats.QuadCount);
				ImGui::Text("Circles: %u", renderer2DStats.CircleCount);
				ImGui::Text("Lines: %u", renderer2DStats.LineCount);
				ImGui::Text("Draw calls: %u", renderer2DStats.DrawCalls);
				ImGui::TextWrapped("Hovered entity: %s", m_HoveredEntity.HasComponents<TagComponent>() ?
					m_HoveredEntity.GetComponents<TagComponent>().Tag.c_str() : "none");
			}

			ImGui::SeparatorText("Memory");
			{
				float currentAllocations;

				if (ImGui::BeginTable("##MemoryUsageStats", 2, ImGuiTableColumnFlags_NoResize | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("Category");
					ImGui::TableSetupColumn("Allocations", ImGuiTableColumnFlags_WidthFixed, 100);
					ImGui::TableHeadersRow();

					for (auto& [file, stats] : allocationStatsMap)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						{
							ImGui::Text("%s", file);
						}
						ImGui::TableSetColumnIndex(1);
						{
							currentAllocations = (float)(stats.TotalAllocated - stats.TotalFreed);

							if (currentAllocations >= BIT(20))
								ImGui::Text(" %.1f MB", currentAllocations / BIT(20));
							else if (currentAllocations >= BIT(10))
								ImGui::Text(" %.1f KB", currentAllocations / BIT(10));
							else
								ImGui::Text(" %.0f bytes", currentAllocations);
						}
					}

					ImGui::PushStyleColor(ImGuiCol_Text, { 0.97f, 0.77f, 0.22f, 1.0f });
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						{
							ImGui::Text("Total");
						}
						ImGui::TableSetColumnIndex(1);
						{
							currentAllocations = (float)(allocationStats.TotalAllocated - allocationStats.TotalFreed);

							if (currentAllocations >= BIT(20))
								ImGui::Text(" %.1f MB", currentAllocations / BIT(20));
							else if (currentAllocations >= BIT(10))
								ImGui::Text(" %.1f KB", currentAllocations / BIT(10));
							else
								ImGui::Text(" %.1f bytes", currentAllocations);
						}
					}
					ImGui::PopStyleColor();

					ImGui::EndTable();
				}
			}

			ImGui::End();
		}		
	}

	void EditorLayer::OnEvent(Event& event)
	{
		if (m_ViewportHovered && (Editor::GetSceneState() == SceneState::Edit || Editor::GetSceneState() == SceneState::Simulate))
			m_EditorCamera.OnEvent(event);

		m_ContentBrowserPanel.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<WindowClosedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnWindowClosed));
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		// TODO: keep adding blocking states
		bool blockKeyInput = ImGuizmo::IsUsing()
			|| m_ShowNewProjectWindow
			|| m_ShowAboutWindow
			|| m_ShowSaveChangesPrompt;

		if (blockKeyInput)
			return false;

		bool ctrl = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

		switch (event.GetKeyCode())
		{
			case KeyCode::S:
			{
				if (Editor::GetSceneState() == SceneState::Edit)
				{
					if (ctrl && shift)
					{
						SaveSceneFileAs();
						return true;
					}
					else if (ctrl)
					{
						SaveSceneFile();
						return true;
					}
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
				if (m_ViewportFocused && Editor::GetSceneState() == SceneState::Edit)
					m_GizmoType = TransformationType::None;

				break;
			}
			case KeyCode::W:
			{
				if (m_ViewportFocused && Editor::GetSceneState() == SceneState::Edit)
					m_GizmoType = TransformationType::Translation;

				break;
			}
			case KeyCode::E:
			{
				if (m_ViewportFocused && Editor::GetSceneState() == SceneState::Edit)
					m_GizmoType = TransformationType::Rotation;

				break;
			}
			case KeyCode::R:
			{
				if (shift && ctrl && Editor::GetSceneState() == SceneState::Edit)
				{
					ScriptEngine::ReloadAssembly();
				}
				else if (m_ViewportFocused && Editor::GetSceneState() == SceneState::Edit)
					m_GizmoType = TransformationType::Scale;

				break;
			}
			case KeyCode::D:
			{
				if (ctrl && Editor::GetSceneState() == SceneState::Edit)
				{
					Entity& selection = Editor::GetSelectedEntity();
					if (selection)
						m_EditorScene->DuplicateEntity(selection);

					return true;
				}

				break;
			}
			case KeyCode::F11:
			{
				Window& window = Application::Get().GetWindow();
				window.SetFullscreen(!window.IsFullscreen());
				return true;

				break;
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		// block other mouse input when we're trying to use imguizmo, or move our camera around
		if (m_EditorCamera.Controlled() || ( ImGuizmo::IsOver() && Editor::GetSelectedEntity() ))
			return false;
		
		if (event.GetMouseButton() == MouseCode::ButtonLeft)
		{
			if (m_ViewportHovered)
				Editor::SelectEntity(m_HoveredEntity);
		}

		return true;
	}

	bool EditorLayer::OnWindowClosed(WindowClosedEvent& event)
	{
		DoAfterHandlingUnsavedChanges([this]()
			{
				if (m_HaveActiveProject)
					SaveProjectFile();

				SaveEditorConfigFile();

				Application::Get().Exit();
			},
			"Save changes before exiting?", false);

		return true;
	}

	void EditorLayer::UISaveChangesPrompt()
	{
		if (m_ShowSaveChangesPrompt)
			ImGui::OpenPopup("Save Changes");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(330, 150));

		if (ImGui::BeginPopupModal("Save Changes", nullptr, ImGuiWindowFlags_NoResize))
		{
			ImGui::SetCursorPosX(.5f * (330 - ImGui::CalcTextSize(m_SaveChangesPromptMessage.c_str()).x));
			ImGui::SetCursorPosY(.5f * (100 - ImGui::CalcTextSize(m_SaveChangesPromptMessage.c_str()).y));
			ImGui::Text(m_SaveChangesPromptMessage.c_str());

			ImGui::SetCursorPosY(80);
			if (ImGui::Button("Save", ImVec2(m_CanCancelSaveChangesPrompt ? 100 : 150, 0)))
			{
				if (SaveSceneFile())
				{
					m_ShowSaveChangesPrompt = false;
					ImGui::CloseCurrentPopup();

					m_AfterSaveChangesCallback();
					m_AfterSaveChangesCallback = []() {};
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Don't Save", ImVec2(m_CanCancelSaveChangesPrompt ? 100 : 150, 0)))
			{
				m_ShowSaveChangesPrompt = false;
				ImGui::CloseCurrentPopup();

				m_AfterSaveChangesCallback();
				m_AfterSaveChangesCallback = []() {};
			}

			if (m_CanCancelSaveChangesPrompt)
			{
				ImGui::SameLine();

				if (ImGui::Button("Cancel", ImVec2(100, 0)))
				{
					m_ShowSaveChangesPrompt = false;
					ImGui::CloseCurrentPopup();

					m_AfterSaveChangesCallback = []() {};
				}
			}

			bool hideThis = !Editor::GetConfig().ShowSavePrompt;
			const char* msg = "Don't show this again";
			ImGui::SetCursorPosX(.5f * (330 - ImGui::CalcTextSize(msg).x));
			ImGui::SetCursorPosY(115);
			if (ImGui::Checkbox(msg, &hideThis))
				Editor::GetConfig().ShowSavePrompt = !hideThis;

			ImGui::EndPopup();
		}
	}

	void EditorLayer::DoAfterHandlingUnsavedChanges(std::function<void()> callback, const std::string& message, bool canCancel)
	{
		if (Editor::GetConfig().ShowSavePrompt)
		{
			// wait for response
			m_ShowSaveChangesPrompt = true;
			m_AfterSaveChangesCallback = callback;
			m_SaveChangesPromptMessage = message;
			m_CanCancelSaveChangesPrompt = canCancel;
		}
		else
		{
			// immediately
			callback();
		}
	}

	void EditorLayer::UINewProjectWindow()
	{
		if (m_ShowSaveChangesPrompt || !m_ShowNewProjectWindow)
			return;

		ImGui::OpenPopup("New Project");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(600, 300));

		if (ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_NoResize))
		{
			ImGui::Text("Project Name");
			ImGui::SetNextItemWidth(-1);
			ImGui::InputTextWithHint("##ProjectNameInput", "Project Name", s_NewProjectNameBuffer, s_NewProjectNameBufferLength);

			ImGui::Text("Parent Directory");
			ImGui::InputTextWithHint("##ProjectLocationInput", "Parent Directory", s_NewProjectLocationBuffer, s_NewProjectLocationBufferLength, ImGuiInputTextFlags_ReadOnly);

			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				std::filesystem::path filepath = FileDialogs::ChooseDirectory();
				
				strcpy_s(s_NewProjectLocationBuffer, s_NewProjectLocationBufferLength, filepath.string().c_str());
			}

			if (ImGui::Button("Generate Project"))
			{
				if (m_HaveActiveProject)
				{
					DoAfterHandlingUnsavedChanges([this]()
						{
							SaveProjectFile();
							SaveEditorConfigFile();

							NewProject(s_NewProjectNameBuffer, s_NewProjectLocationBuffer);
						},
						"Save current scene?", true);
				}
				else
				{
					NewProject(s_NewProjectNameBuffer, s_NewProjectLocationBuffer);
				}

				m_ShowNewProjectWindow = false;
			}

			if (m_HaveActiveProject)
			{
				//ImGui::SameLine();

				if (ImGui::Button("Cancel"))
				{
					m_ShowNewProjectWindow = false;
				}
			}

			ImGui::EndPopup();
		}
	}

	void EditorLayer::NewProject(const std::string& projectName, const std::filesystem::path& parentDirectory)
	{
		auto project = Project::New();
		auto& config = project->GetConfig();

		// Create project directory
		config.ProjectName = projectName;
		config.ProjectDirectory = parentDirectory / projectName;
		if (!std::filesystem::exists(config.ProjectDirectory))
			std::filesystem::create_directory(config.ProjectDirectory);

		// TODO: move this to AssetManager creation!!
		// Create asset subdirectories
		config.AssetDirectory = "Assets";
		if (!std::filesystem::exists(Project::GetAssetsDirectory()))
			std::filesystem::create_directory(Project::GetAssetsDirectory());
		{
			if (!std::filesystem::exists(Project::GetFontsDirectory()))
				std::filesystem::create_directory(Project::GetFontsDirectory());

			if (!std::filesystem::exists(Project::GetMeshesDirectory()))
				std::filesystem::create_directory(Project::GetMeshesDirectory());

			if (!std::filesystem::exists(Project::GetScenesDirectory()))
				std::filesystem::create_directory(Project::GetScenesDirectory());

			if (!std::filesystem::exists(Project::GetScriptsDirectory()))
				std::filesystem::create_directory(Project::GetScriptsDirectory());

			if (!std::filesystem::exists(Project::GetTexturesDirectory()))
				std::filesystem::create_directory(Project::GetTexturesDirectory());

			// TODO: add more asset types as they're introduced
			// (icons, audio, animations, sprite sheets, custom shaders etc.)
		}

		// TODO: create directory tree for runtime C++ source files

		// TODO: create any files we want every project to have (may depend on OS, compiler, etc.):
		//		1) "premake5.lua" files for project C++, script C# library etc.
		//		2) Shell scripts that actually run premake on those files (create then immediately execute these)
		//		3) Generic assets?
		//		4) An "imgui.ini" with defaults filled in (for runtime debug, console commands etc.)

		// Create project file
		config.ProjectFilepath = config.ProjectDirectory / (projectName + ".zpj");
		Project::Save(config.ProjectFilepath);

		// Propogate new project to the editor subsystems
		m_WorkingProjectFilepath = config.ProjectFilepath;
		m_HaveActiveProject = true;
		m_ContentBrowserPanel.OnLoadProject();

		// Create default starting scene
		NewScene();
		config.StartingSceneFilepath = "Assets/Scenes/" + projectName + "_starting_scene.zsc";
		if (!std::filesystem::exists(Project::GetStartingSceneFilepath()))
			SaveSceneFileAs(config.ProjectDirectory / config.StartingSceneFilepath);

		memset(s_NewProjectNameBuffer, 0, sizeof(s_NewProjectNameBuffer));
		memset(s_NewProjectLocationBuffer, 0, sizeof(s_NewProjectLocationBuffer));
	}

	void EditorLayer::OpenProjectFile()
	{
		std::filesystem::path filepath = FileDialogs::OpenFile(s_ProjectFilter);
		if (filepath.empty())
			return;

		DoAfterHandlingUnsavedChanges([this, filepath]()
			{
				OpenProjectFile(filepath);
			},
			"Save current scene?", true);
	}

	bool EditorLayer::OpenProjectFile(const std::filesystem::path& filepath)
	{
		if (filepath.empty() || !std::filesystem::exists(filepath))
			return false;

		if (Project::Load(filepath))
		{
			m_WorkingProjectFilepath = filepath;
			m_ContentBrowserPanel.OnLoadProject();

			m_WorkingSceneRelativePath = Project::GetStartingSceneFilepath();
			auto workingScene = Project::GetProjectDirectory() / m_WorkingSceneRelativePath;
			if (!OpenSceneFile(workingScene))
				NewScene();

			return true;
		}

		return false;
	}

	void EditorLayer::SaveProjectFile()
	{
		Z_CORE_ASSERT(!m_WorkingProjectFilepath.empty());

		Project::Save(m_WorkingProjectFilepath);
	}

	bool EditorLayer::TryLoadProjectScriptAssembly(const std::filesystem::path& filepath)
	{
		Z_CORE_ASSERT(m_HaveActiveProject);

		if (filepath.empty())
			return false;

		
		auto relativePath = std::filesystem::relative(filepath, Project::GetProjectDirectory());
		if (relativePath.empty())
			return false;

		auto& config = Project::GetActive()->GetConfig();

		if (ScriptEngine::InitApp(filepath, config.AutoReloadScriptAssembly))
		{
			config.ScriptAssemblyFilepath = relativePath;
			return true;
		}

		return false;
	}

	void EditorLayer::NewScene()
	{
		Z_CORE_ASSERT(m_HaveActiveProject);

		m_AutosaveEnabled = false;

		if (Editor::GetSceneState() != SceneState::Edit)
			SceneStop();

		m_HoveredEntity = {};
		m_EditorScene = Ref<Scene>::Create();

		m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
		m_WorkingSceneRelativePath.clear();

		m_ActiveScene = m_EditorScene;
		Editor::SetSceneContext(m_ActiveScene);
	}

	void EditorLayer::OpenSceneFile()
	{
		Z_CORE_ASSERT(m_HaveActiveProject);

		std::filesystem::path filepath = FileDialogs::OpenFile(s_SceneFilter);
		if (filepath.empty())
			return;

		DoAfterHandlingUnsavedChanges([this, filepath]()
			{
				OpenSceneFile(filepath);
			},
			"Save current scene?", true);
	}

	bool EditorLayer::OpenSceneFile(std::filesystem::path filepath)
	{
		Z_CORE_ASSERT(m_HaveActiveProject);

		const auto& projectDirectory = Project::GetProjectDirectory();
		auto relativeFilepath = std::filesystem::relative(filepath, projectDirectory);
		if (relativeFilepath.empty())
		{
			Z_CORE_WARN("Attempted to load '{0}', outside of the current project directory '{1}'",
				filepath.string(), projectDirectory.string());
			return false;
		}

		if (filepath.empty() || !std::filesystem::exists(filepath))
			return false;

		if (Editor::GetSceneState() != SceneState::Edit)
			SceneStop();

		m_HoveredEntity = {};

		if (filepath.extension().string() != ".zsc")
		{
			Z_WARN("Couldn't open {0} - scene files must have extension '.zsc'", filepath.filename().string());
			return false;
		}

		Ref<Scene> newScene = Ref<Scene>::Create();
		SceneSerialiser serialiser(newScene);
		if (serialiser.DeserialiseYaml(filepath.string()))
		{
			m_EditorScene = newScene;

			m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);

			std::string sceneName = filepath.filename().string();
			m_EditorScene->SetName(sceneName);

			m_WorkingSceneRelativePath = relativeFilepath;			

			m_ActiveScene = m_EditorScene;
			Editor::SetSceneContext(m_EditorScene);

			m_AutosaveTimer.Reset();
			m_AutosaveEnabled = Editor::GetConfig().AutosaveInterval > 0;

			SaveProjectFile();
			SaveEditorConfigFile();

			return true;
		}

		return false;
	}

	bool EditorLayer::SaveSceneFile()
	{
		Z_CORE_ASSERT(m_HaveActiveProject);

		if (m_WorkingSceneRelativePath.empty())
			return SaveSceneFileAs();

		SceneSerialiser serialiser(m_EditorScene);
		auto filepath = Project::GetProjectDirectory() / m_WorkingSceneRelativePath;
		serialiser.SerialiseYaml(filepath.string());

		std::string sceneName = m_WorkingSceneRelativePath.filename().string();
		m_EditorScene->SetName(sceneName);

		SaveProjectFile();
		SaveEditorConfigFile();
		
		return true;
	}

	bool EditorLayer::SaveSceneFileAs()
	{
		Z_CORE_ASSERT(m_HaveActiveProject);

		std::filesystem::path filepath = FileDialogs::SaveFile(s_SceneFilter);
		if (filepath.empty())
			return false;

		return SaveSceneFileAs(filepath);
	}

	bool EditorLayer::SaveSceneFileAs(const std::filesystem::path& filepath)
	{
		const auto& projectDirectory = Project::GetProjectDirectory();
		auto relativeFilepath = std::filesystem::relative(filepath, projectDirectory);
		if (relativeFilepath.empty())
		{
			Z_CORE_WARN("Attempted to save scene to '{0}', outside of the current project directory '{1}'",
				filepath.string(), projectDirectory.string());
			return false;
		}

		std::string sceneName = filepath.filename().string();
		m_EditorScene->SetName(sceneName);

		m_WorkingSceneRelativePath = relativeFilepath;

		SceneSerialiser serialiser(m_EditorScene);
		serialiser.SerialiseYaml(filepath.string());

		SaveProjectFile();
		SaveEditorConfigFile();

		m_AutosaveTimer.Reset();
		m_AutosaveEnabled = Editor::GetConfig().AutosaveInterval > 0;

		return true;
	}

	void EditorLayer::SaveEditorConfigFile()
	{
		if (!m_HaveActiveProject)
			return;

		auto& config = Editor::GetConfig();

		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			
			out << YAML::Key << "WorkingProjectFilepath";
			out << YAML::Value << m_WorkingProjectFilepath.string();

			// TODO: replace with scene asset id
			if (!m_WorkingSceneRelativePath.empty())
			{
				out << YAML::Key << "WorkingSceneFilepath";
				out << YAML::Value << m_WorkingSceneRelativePath.string();
			}

			out << YAML::Key << "AutosaveInterval";
			out << YAML::Value << config.AutosaveInterval;

			out << YAML::Key << "MaxCachedScenes";
			out << YAML::Value << config.MaxCachedScenes;

			out << YAML::Key << "ShowSavePrompt";
			out << YAML::Value << config.ShowSavePrompt;

			out << YAML::Key << "FramesPerStep";
			out << YAML::Value << m_FramesPerStep;
		}
		out << YAML::EndMap;

		std::filesystem::path configDirectory = Application::Get().GetSpecification().WorkingDirectory / "Config";
		if (!std::filesystem::exists(configDirectory))
			std::filesystem::create_directories(configDirectory);
		std::filesystem::path configFilepath = configDirectory / "editor_config.yml";

		std::ofstream fout(configFilepath.c_str(), std::ios_base::out);
		fout << out.c_str();
		fout.close();
	}

	void EditorLayer::LoadConfigFile()
	{
		auto& config = Editor::GetConfig();

		std::filesystem::path configDirectory = Application::Get().GetSpecification().WorkingDirectory / "Config";
		std::filesystem::path configFilepath = configDirectory / "editor_config.yml";
		if (!std::filesystem::exists(configFilepath))
			return;

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(configFilepath.string());
		}
		catch (const YAML::ParserException& ex)
		{
			Z_CORE_ERROR("Failed to load editor configuration file '{0}':\n{1}", configFilepath.filename().string(), ex.what());
		}

		if (auto workingProjectNode = data["WorkingProjectFilepath"])
		{
			m_WorkingProjectFilepath = workingProjectNode.as<std::string>();
		}

		// TODO: replace with scene asset id
		if (auto workingSceneNode = data["WorkingSceneFilepath"])
		{
			m_WorkingSceneRelativePath = workingSceneNode.as<std::string>();
		}

		if (auto sceneCacheIntervalNode = data["AutosaveInterval"])
		{
			config.AutosaveInterval = sceneCacheIntervalNode.as<float>();
		}

		if (auto maxCachedScenesNode = data["MaxCachedScenes"])
		{
			config.MaxCachedScenes = maxCachedScenesNode.as<uint32_t>();
		}

		if (auto showSavePromptNode = data["ShowSavePrompt"])
		{
			config.ShowSavePrompt = showSavePromptNode.as<bool>();
		}

		if (auto framesPerStepNode = data["FramesPerStep"])
		{
			m_FramesPerStep = framesPerStepNode.as<int32_t>();
		}
	}

	void EditorLayer::ReadHoveredEntity()
	{
		if (!m_HaveActiveProject)
			return;

		// TODO: compare performance: reading IDs from a framebuffer attachment vs CPU-side raycasting

		ImVec2 mouse = ImGui::GetMousePos();
		mouse.x -= m_ViewportBounds[0].x;
		mouse.y -= m_ViewportBounds[0].y;

		void* pixelAddress = m_ColourPickingAttachment->ReadPixel((int)mouse.x, (int)mouse.y);
		int32_t hoveredID = pixelAddress ? *((int32_t*)pixelAddress) : -1;
		m_HoveredEntity = (hoveredID == -1) ? Entity() : Entity((entt::entity)hoveredID, m_ActiveScene.Raw());
	}	
}

