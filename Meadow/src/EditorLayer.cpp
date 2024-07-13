#include "EditorLayer.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>


namespace Zahra
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
		//m_CameraController(1280.0f / 720.0f)
	{
	}

	void EditorLayer::OnAttach()
	{
		m_Texture = Texture2D::Create("C:/dev/Zahra/Sandbox/assets/textures/yajirobe.png");

		FramebufferSpecification framebufferSpec;
		framebufferSpec.Width = 1280;
		framebufferSpec.Height = 720; // These will be overwritten by the ImGui viewport window

		m_Framebuffer = Framebuffer::Create(framebufferSpec);

		m_ActiveScene = CreateRef<Scene>();

		m_QuadEntity = m_ActiveScene->CreateEntity("a_quad");
		m_QuadEntity.AddComponent<SpriteComponent>();

		m_FixedCamera = m_ActiveScene->CreateEntity("fixed_camera");
		m_FixedCamera.AddComponent<CameraComponent>(false);

		// TODO: obviously this belongs elsewhere
		class CameraController : public ScriptableEntity
		{
		public:
			void OnUpdate(float dt)
			{
				auto& transform = GetComponents<TransformComponent>().Transform;
				auto& camera = GetComponents<CameraComponent>().Camera;
				float speed = camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic
					? .5f * camera.GetOrthographicSize() : 2.0f;
				
				if (Input::IsKeyPressed(KeyCode::A))
					transform[3][0] -= speed * dt;
				if (Input::IsKeyPressed(KeyCode::D))
					transform[3][0] += speed * dt;
				if (Input::IsKeyPressed(KeyCode::W))
					transform[3][1] += speed * dt;
				if (Input::IsKeyPressed(KeyCode::S))
					transform[3][1] -= speed * dt;
			}
		};

		m_DynamicCamera = m_ActiveScene->CreateEntity("dynamic_camera");
		m_DynamicCamera.AddComponent<CameraComponent>(false);
		m_DynamicCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		m_DynamicCamera.GetComponents<CameraComponent>().active = false;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(float dt)
	{
		m_FPS = 1.0f / dt;

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

				m_QuadEntity.GetComponents<SpriteComponent>().Colour = glm::make_vec4(m_QuadColour);
				m_QuadEntity.GetComponents<TransformComponent>().Transform =
					glm::translate(glm::mat4(1.0f), glm::make_vec3(m_QuadPosition))
					* glm::rotate(glm::mat4(1.0f), m_QuadRotation, glm::vec3(.0f, .0f, 1.0f))
					* glm::scale(glm::mat4(1.0f), glm::make_vec3(m_QuadDimensions));

				m_FixedCamera.GetComponents<CameraComponent>().active = m_CameraToggle;
				m_DynamicCamera.GetComponents<CameraComponent>().active = !m_CameraToggle;

				m_ActiveScene->OnUpdate(dt);
			}
			m_Framebuffer->Unbind();
			
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	}

	void EditorLayer::OnEvent(Event& event)
	{
		//m_CameraController.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(Z_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
	}

	void EditorLayer::OnImGuiRender()
	{
		Z_PROFILE_FUNCTION();

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

		// TODO: get rid of windows below (fold some functions into others)
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RENDERING DEMO CONTROLS WINDOW
		{
			ImGui::Begin("Demo Controls");

			ImGui::Text("Background:");
			ImGui::ColorEdit3("Colour", m_ClearColour);

			ImGui::Separator();

			ImGui::Text("Quad:");
			ImGui::SliderFloat3("Position", m_QuadPosition, -5.0f, 5.0f);
			ImGui::SliderFloat2("Size", m_QuadDimensions, .01f, 10.0f, "%.2f", 32);
			ImGui::SliderFloat("Rotation", &m_QuadRotation, -3.14f, 3.14f);
			ImGui::ColorEdit4("Tint", m_QuadColour);

			ImGui::Checkbox("Use fixed camera", &m_CameraToggle);

			ImGui::End();
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RENDERING STATS WINDOW
		{
			ImGui::Begin("Renderer Stats");

			ImGui::Text("FPS: %i", (int)m_FPS);
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

