#include "EditorLayer.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>


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

		m_Quad = m_ActiveScene->CreateEntity("example quad");
		m_Quad.AddComponent<SpriteComponent>();
		m_Quad.GetComponents<SpriteComponent>().Colour = { .878f, .718f, .172f, 1.0f };
		m_Quad.GetComponents<TransformComponent>().Translation = { .0f, .0f, -.5f };
		m_Quad.GetComponents<TransformComponent>().Scale = { 1.0f, 1.0f, 1.0f };

		// TODO: obviously this belongs elsewhere
		class CameraController : public ScriptableEntity
		{
		public:
			void OnUpdate(float dt)
			{
				auto& position = GetComponents<TransformComponent>().Translation;
				if (HasComponents<CameraComponent>())
				{
					auto& camera = GetComponents<CameraComponent>().Camera;
					float speed = camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic
						? .5f * camera.GetOrthographicSize() : 2.0f;

					if (Input::IsKeyPressed(KeyCode::A))
						position.x -= speed * dt;
					if (Input::IsKeyPressed(KeyCode::D))
						position.x += speed * dt;
					if (Input::IsKeyPressed(KeyCode::W))
						position.y += speed * dt;
					if (Input::IsKeyPressed(KeyCode::S))
						position.y -= speed * dt;
				}
			}
		};

		m_Camera = m_ActiveScene->CreateEntity("example camera");
		m_Camera.AddComponent<CameraComponent>();
		m_Camera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		m_Camera.GetComponents<CameraComponent>().Active = true;

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
					if (ImGui::MenuItem("Exit")) Application::Get().Exit();

					ImGui::EndMenu();
				}

			}
			ImGui::EndMainMenuBar();

		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// WINDOW DOCKSPACE
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// VIEWPORT
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
			ImGui::Begin("Viewport");

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
			ImGui::Begin("Stats");

			ImGui::Text("Quads: %u", Renderer2D::GetStats().QuadCount);
			ImGui::Text("Draw calls: %u", Renderer2D::GetStats().DrawCalls);

			ImGui::End();
		}

	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		return false;
	}


}

