#include "EditorLayer.h"

#include "Editor/Editor.h"
#include "Zahra/Maths/Maths.h"
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
	EditorLayer::EditorLayer()
		: Layer("EditorLayer") {}

	void EditorLayer::OnAttach()
	{
		ReadConfigFile();
		
		auto imguiLayer = ImGuiLayer::GetOrCreate();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// VIEWPORT FRAMEBUFFER
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
				attachment.Format = ImageFormat::SRGBA;
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

			/*auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
			if (commandLineArgs.Count > 1)
			{
				auto sceneFilePath = commandLineArgs[1];
				SceneSerialiser serialiser(m_EditorScene);
				serialiser.DeserialiseYaml(sceneFilePath);
			}*/

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

		OpenSceneFile(m_WorkingSceneFilepath);
		m_SceneCacheTimer.Reset();

		Editor::Reset();
	}

	void EditorLayer::OnDetach()
	{
		WriteConfigFile();

		m_Renderer2D.Reset();
		m_ActiveScene.Reset();
		m_EditorScene.Reset();

		m_ClearPass.Reset();

		ImGuiLayer::GetOrCreate()->DeregisterTexture(m_ViewportTextureHandle);
		m_ViewportTextureHandle = nullptr;
		m_ViewportTexture.Reset();
		m_ViewportFramebuffer.Reset();

		Editor::Reset();
	}

	void EditorLayer::OnUpdate(float dt)
	{
		if (m_FramerateRefreshTimer.Elapsed() >= c_FramerateRefreshInterval)
		{
			m_FramerateRefreshTimer.Reset();
			m_Framerate = 1.0f / dt;
		}

		if (m_SceneCacheTimer.Elapsed() > Editor::GetConfig().SceneCacheInterval)
		{
			m_SceneCacheTimer.Reset();
			CacheWorkingScene();
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

			switch (m_SceneState)
			{
				case SceneState::Edit:
				{
					m_ActiveScene->OnUpdateEditor(dt);
					m_ActiveScene->OnRenderEditor(m_Renderer2D, m_EditorCamera, m_SceneHierarchyPanel.GetSelectedEntity(), m_HighlightSelectionColour);
					break;
				}
				case SceneState::Play:
				{
					m_ActiveScene->OnUpdateRuntime(dt);
					m_ActiveScene->OnRenderRuntime(m_Renderer2D, m_SceneHierarchyPanel.GetSelectedEntity(), m_HighlightSelectionColour);
					break;
				}
				case SceneState::Simulate:
				{
					m_ActiveScene->OnUpdateSimulation(dt);
					m_ActiveScene->OnRenderEditor(m_Renderer2D, m_EditorCamera, m_SceneHierarchyPanel.GetSelectedEntity(), m_HighlightSelectionColour);
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
		UIControls();
		UIStatsWindow();

		m_SceneHierarchyPanel.OnImGuiRender(m_SceneState);
		m_ContentBrowserPanel.OnImGuiRender();

		UISaveChangesPrompt();
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
				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenSceneFile();

				ImGui::Separator();

				if (ImGui::MenuItem("Save", "Ctrl+S"))					
					SaveSceneFile();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveAsSceneFile();

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
				{
					DoAfterHandlingUnsavedChanges([]()
						{
							Application::Get().Exit();
						});
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
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
				ImGui::SeparatorText("Transform Gizmo");

				if (ImGui::MenuItem("Select", "Q"))
					m_GizmoType = -1;

				if (ImGui::MenuItem("Translate", "W"))
					m_GizmoType = 0;

				if (ImGui::MenuItem("Rotate", "E"))
					m_GizmoType = 1;

				if (ImGui::MenuItem("Scale", "R"))
					m_GizmoType = 2;

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

			if (m_SceneState == SceneState::Edit)
				UIGizmo();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BROWSER_FILE_SCENE"))
				{
					char filepath[256];
					strcpy_s(filepath, (const char*)payload->Data);

					DoAfterHandlingUnsavedChanges([this, filepath]()
						{
							OpenSceneFile(filepath);
						});
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}
	}

	void EditorLayer::UIGizmo()
	{
		Entity selection = m_SceneHierarchyPanel.GetSelectedEntity();
		if (!selection || m_GizmoType == -1)
			return;
		
		// Configure ImGuizmo
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);
			ImGuizmo::SetGizmoSizeClipSpace(.15f);

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


		// Gather camera details
		glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
		cameraProjection[1][1] *= -1.f;
		glm::mat4 cameraView = m_EditorCamera.GetView();

		// Get current transform values
		auto& transformComponent = selection.GetComponents<TransformComponent>();
		glm::mat4 transform = transformComponent.GetTransform();

		// Configure snapping
		bool snap = Input::IsKeyPressed(Key::LeftControl);
		float snapValue = (m_GizmoType == 120) ? 45.0f : 0.5f;
		float snapVector[3] = { snapValue, snapValue, snapValue };

		// Pass data to ImGuizmo
		ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType,
			ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapVector : nullptr);

		// Feedback manipulated transform, and cache transform for undo/redo actions
		if (ImGuizmo::IsUsing() && !m_EditorCamera.Controlled())
		{
			if (!m_GizmoWasUsedLastFrame)
				m_CachedTransform = transformComponent;

			glm::vec3 eulers;
			Maths::DecomposeTransform(transform, transformComponent.Translation, eulers, transformComponent.Scale);
			transformComponent.SetRotation(eulers);
		}
		else
		{
			if (m_GizmoWasUsedLastFrame)
			{
				if (m_CachedTransform.Translation != transformComponent.Translation)
				{
					glm::vec3 oldTranslation = m_CachedTransform.Translation;
					glm::vec3 newTranslation = transformComponent.Translation;

					EditAction gizmoTranslate;
					gizmoTranslate.Do = [newTranslation, &transformComponent]()
						{
							transformComponent.Translation = newTranslation;
						};
					gizmoTranslate.Undo = [oldTranslation, &transformComponent]()
						{
							transformComponent.Translation = oldTranslation;
						};

					Editor::NewAction(gizmoTranslate);
				}
				else if (m_CachedTransform.GetEulers() != transformComponent.GetEulers())
				{
					glm::vec3 oldEulers = m_CachedTransform.GetEulers();
					glm::vec3 newEulers = transformComponent.GetEulers();

					EditAction gizmoRotate;
					gizmoRotate.Do = [newEulers, &transformComponent]()
						{
							transformComponent.SetRotation(newEulers);
						};
					gizmoRotate.Undo = [oldEulers, &transformComponent]()
						{
							transformComponent.SetRotation(oldEulers);
						};

					Editor::NewAction(gizmoRotate);
				}
				else if (m_CachedTransform.Scale != transformComponent.Scale)
				{
					glm::vec3 oldScale = m_CachedTransform.Scale;
					glm::vec3 newScale = transformComponent.Scale;

					EditAction gizmoScale;
					gizmoScale.Do = [newScale, &transformComponent]()
						{
							transformComponent.Scale = newScale;
						};
					gizmoScale.Undo = [oldScale, &transformComponent]()
						{
							transformComponent.Scale = oldScale;
						};

					Editor::NewAction(gizmoScale);
				}
			}
		}

		m_GizmoWasUsedLastFrame = ImGuizmo::IsUsing();
	}

	// TODO: move highight rendering to Scene, or SceneRenderer
	//void EditorLayer::UIHighlightSelection()
	//{
	//	
	//}

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

	void EditorLayer::UISaveChangesPrompt()
	{
		if (m_ShowSaveChangesPrompt)
			ImGui::OpenPopup("Unsaved Changes");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(330, 120));

		if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_NoResize))
		{
			// TODO: change from scene to project?
			ImGui::SetCursorPosY(40);
			ImGui::Text("The current scene has unsaved changes.\nWould you like to save before exiting?");

			ImGui::SetCursorPosY(80);
			if (ImGui::Button("Save", ImVec2(100, 0)))
			{
				if (SaveSceneFile())
				{
					m_ShowSaveChangesPrompt = false;
					ImGui::CloseCurrentPopup();

					m_SaveChangesCallback();
					m_SaveChangesCallback = []() {};
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Don't Save", ImVec2(100, 0)))
			{
				m_ShowSaveChangesPrompt = false;
				ImGui::CloseCurrentPopup();

				m_SaveChangesCallback();
				m_SaveChangesCallback = []() {};
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(100, 0)))
			{
				m_ShowSaveChangesPrompt = false;
				ImGui::CloseCurrentPopup();

				m_SaveChangesCallback = []() {};
			}

			ImGui::EndPopup();
		}
	}

	void EditorLayer::DoAfterHandlingUnsavedChanges(std::function<void()> callback)
	{
		if (Editor::UnsavedChanges())
		{
			m_ShowSaveChangesPrompt = true;
			m_SaveChangesCallback = callback;
		}
		else
			callback();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		if (m_ViewportHovered && m_SceneState != SceneState::Play)
			m_EditorCamera.OnEvent(event);

		m_ContentBrowserPanel.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<WindowClosedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnWindowClosed));
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Keyboard shortcuts
		
		if (ImGuizmo::IsUsing())
			return false; // here be dragons

		bool ctrl = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

		switch (event.GetKeyCode())
		{
			case KeyCode::Z:
			{
				if (ctrl)
				{
					Editor::Undo();
					return true;
				}

				break;
			}
			case KeyCode::Y:
			{
				if (ctrl)
				{
					Editor::Redo();
					return true;
				}

				break;
			}
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
				if (m_ViewportFocused && m_SceneState == SceneState::Edit)
					m_GizmoType = -1;

				break;
			}
			case KeyCode::W:
			{
				if (m_ViewportFocused && m_SceneState == SceneState::Edit)
					m_GizmoType = 7;

				break;
			}
			case KeyCode::E:
			{
				if (m_ViewportFocused && m_SceneState == SceneState::Edit)
					m_GizmoType = 120;

				break;
			}
			case KeyCode::R:
			{
				if (m_ViewportFocused && m_SceneState == SceneState::Edit)
					m_GizmoType = 896;

				break;
			}
			case KeyCode::D:
			{
				if (ctrl && m_SceneState == SceneState::Edit)
				{
					Entity& selection = m_SceneHierarchyPanel.GetSelectedEntity();
					if (selection)
					{
						/*Entity copy = m_EditorScene->DuplicateEntity(selection);
						m_SceneHierarchyPanel.SelectEntity(copy);*/

						ZGUID extantID = selection.GetGUID();
						ZGUID newID;

						EditAction duplicateEntity;
						duplicateEntity.Do = [&, extantID, newID]()
							{
								m_EditorScene->DuplicateEntity(m_EditorScene->GetEntity(extantID), newID);
							};
						duplicateEntity.Undo = [&, newID]()
							{
								if (Entity select = m_SceneHierarchyPanel.GetSelectedEntity())
								{
									if (select.GetGUID() == newID)
										m_SceneHierarchyPanel.SelectEntity({});
								}

								m_EditorScene->DestroyEntity(newID);
							};
						Editor::NewAction(duplicateEntity);

						m_SceneHierarchyPanel.SelectEntity(m_EditorScene->GetEntity(newID));
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

				break;
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

	bool EditorLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event)
	{
		if (event.GetMouseButton() == MouseCode::ButtonLeft)
		{
			
		}

		return false;
	}

	bool EditorLayer::OnWindowClosed(WindowClosedEvent& event)
	{
		DoAfterHandlingUnsavedChanges([]()
			{
				Application::Get().Exit();
			});

		return true;
	}

	void EditorLayer::NewScene()
	{
		if (m_SceneState != SceneState::Edit)
			SceneStop();

		m_EditorScene = Ref<Scene>::Create();

		m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
		m_WorkingSceneFilepath.clear();

		m_ActiveScene = m_EditorScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		Editor::Reset();
	}

	void EditorLayer::OpenSceneFile()
	{
		std::filesystem::path filepath = FileDialogs::OpenFile(m_FileTypesFilter[0], m_FileTypesFilter[1]);
		DoAfterHandlingUnsavedChanges([this, filepath]()
			{
				OpenSceneFile(filepath);
			});
	}

	void EditorLayer::OpenSceneFile(std::filesystem::path filepath)
	{
		if (filepath.empty())
			return;
		
		if (m_SceneState != SceneState::Edit)
			SceneStop();

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
			m_WorkingSceneFilepath = filepath;

			m_ActiveScene = m_EditorScene;
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);

			Editor::Reset();
		}

		// TODO: report/display success of file open
	}

	bool EditorLayer::SaveSceneFile()
	{
		if (m_WorkingSceneFilepath.empty())
			return SaveAsSceneFile();
		
		std::string sceneName = m_WorkingSceneFilepath.filename().string();
		m_EditorScene->SetName(sceneName);

		SceneSerialiser serialiser(m_EditorScene);
		serialiser.SerialiseYaml(m_WorkingSceneFilepath.string());
		
		WriteConfigFile();
		Editor::OnSave();
		return true;
		// TODO: report/display success of file save
	}

	bool EditorLayer::SaveAsSceneFile()
	{
		std::filesystem::path filepath = FileDialogs::SaveFile(m_FileTypesFilter[0], m_FileTypesFilter[1]);
		if (!filepath.empty())
		{
			std::string sceneName = filepath.filename().string();
			m_EditorScene->SetName(sceneName);
			m_WorkingSceneFilepath = filepath;

			SceneSerialiser serialiser(m_EditorScene);
			serialiser.SerialiseYaml(m_WorkingSceneFilepath.string());

			WriteConfigFile();
			Editor::OnSave();
			return true;
		}

		return false;

		// TODO: report/display success of file save
	}

	void EditorLayer::WriteConfigFile()
	{
		auto& config = Editor::GetConfig();

		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			// TODO: replace with WorkingProjectFilepath!!
			out << YAML::Key << "WorkingSceneFilepath";
			out << YAML::Value << m_WorkingSceneFilepath.string();

			out << YAML::Key << "SceneCacheInterval";
			out << YAML::Value << config.SceneCacheInterval;

			out << YAML::Key << "MaxCachedScenes";
			out << YAML::Value << config.MaxCachedScenes;
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

	void EditorLayer::ReadConfigFile()
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

		// TODO: replace with WorkingProjectFilepath!!
		if (auto workingSceneNode = data["WorkingSceneFilepath"])
		{
			m_WorkingSceneFilepath = workingSceneNode.as<std::string>();
		}

		if (auto sceneCacheIntervalNode = data["SceneCacheInterval"])
		{
			config.SceneCacheInterval = sceneCacheIntervalNode.as<float>();
		}

		if (auto maxCachedScenesNode = data["MaxCachedScenes"])
		{
			config.MaxCachedScenes = maxCachedScenesNode.as<uint32_t>();
		}
	}

	void EditorLayer::CacheWorkingScene()
	{
		// occasionally make a backup save of the working scene, for debugging/recovery
		std::string cacheFilename = "scene_cache_" + std::to_string(m_SceneCacheIndex) + ".zsc";
		std::filesystem::path cache = "Cache/Scene";
		if (!std::filesystem::exists(cache))
			std::filesystem::create_directories(cache);
		cache /= cacheFilename;

		SceneSerialiser serialiser(m_EditorScene);
		serialiser.SerialiseYaml(cache.string());

		m_SceneCacheIndex = (m_SceneCacheIndex + 1) % Editor::GetConfig().MaxCachedScenes;
	}

	/*void EditorLayer::DeleteBackup()
	{
		std::filesystem::path backup = Editor::GetConfig().BackupFilepath;
		if (!std::filesystem::exists(backup))
			return;

		try
		{
			std::filesystem::remove(backup);
		}
		catch (const std::filesystem::filesystem_error& err)
		{
			Z_CORE_ERROR("filesystem error: {}", err.what());
		}
	}*/

	void EditorLayer::ReadHoveredEntity()
	{
		// TODO: compare performance: reading IDs from a framebuffer attachment vs CPU-side raycasting

		ImVec2 mouse = ImGui::GetMousePos();
		mouse.x -= m_ViewportBounds[0].x;
		mouse.y -= m_ViewportBounds[0].y;

		int32_t hoveredID = *((int32_t*)m_ColourPickingAttachment->ReadPixel((int)mouse.x, (int)mouse.y));
		m_HoveredEntity = (hoveredID == -1) ? Entity() : Entity((entt::entity)hoveredID, m_ActiveScene.Raw());
	}

	
}

