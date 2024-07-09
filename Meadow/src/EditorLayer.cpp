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

		m_QuadEntity = m_ActiveScene->CreateEntity("quad1");
		m_QuadEntity.AddComponent<SpriteComponent>();

		m_CameraEntity = m_ActiveScene->CreateEntity("camera1");
		CameraComponent camera = m_CameraEntity.AddComponent<CameraComponent>(false);

	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(float dt)
	{
		Z_PROFILE_FUNCTION();

		m_FPS = 1.0f / dt;

		{
			Z_PROFILE_SCOPE("Resize framebuffer");

			FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f // framebuffer requires positive dimensions
				&& (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
			{
				m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				
				//m_CameraController.Resize(m_ViewportSize.x, m_ViewportSize.y);
				
				m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
			}
		}

		{
			Z_PROFILE_SCOPE("Camera update");

			//if (m_ViewportFocused) m_CameraController.OnUpdate(dt);
		}

		// Rendering (TODO: this should be inside the Scene's OnUpdate())
		{
			Renderer2D::ResetStats();

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// DRAW TO FRAMEBUFFER
			m_Framebuffer->Bind();
			{
				RenderCommand::SetClearColour(glm::make_vec4(m_ClearColour));
				RenderCommand::Clear();

				m_QuadEntity.GetComponents<SpriteComponent>().Colour = glm::make_vec4(m_QuadColour);
				m_QuadEntity.GetComponents<TransformComponent>().Transform =
					glm::translate(glm::mat4(1.0f), glm::make_vec3(m_QuadPosition))
					* glm::rotate(glm::mat4(1.0f), m_QuadRotation, glm::vec3(.0f, .0f, 1.0f))
					* glm::scale(glm::mat4(1.0f), glm::make_vec3(m_QuadDimensions));

				m_CameraEntity.GetComponents<CameraComponent>().Camera.SetOrthographicSize(10.0f / m_Zoom);

				m_ActiveScene->OnUpdate(dt);
			}
			m_Framebuffer->Unbind();
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		}

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
			// RENDERING DEMO CONTROLS WINDOW
		{
			ImGui::Begin("Demo Controls");

			ImGui::Text("Background:");
			ImGui::ColorEdit3("Colour", m_ClearColour);

			ImGui::Separator();

			ImGui::Text("Quad:");
			ImGui::SliderFloat2("Position (x,y)", m_QuadPosition, -5.0f, 5.0f);
			ImGui::SliderFloat2("Size (w,h)", m_QuadDimensions, .01f, 10.0f, "%.2f", 32);
			ImGui::SliderFloat("Rotation", &m_QuadRotation, -3.14f, 3.14f);
			ImGui::ColorEdit4("Tint", m_QuadColour);

			ImGui::Separator();

			ImGui::Text("Camera:");
			ImGui::SliderFloat("Zoom", &m_Zoom, .1f, 10.0f, "%.2f", 32);

			ImGui::End();
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RENDERING STATS WINDOW
		{
			ImGui::Begin("Renderer Stats");

			ImGui::Text("FPS: %i", (int)m_FPS);
			ImGui::Text("Quads: %u", Renderer2D::GetStats().QuadCount);
			ImGui::Text("Draw calls: %u", Renderer2D::GetStats().DrawCalls);

			if (m_QuadEntity)
			{
				ImGui::Separator();
				ImGui::Text("ACTIVE ENTITIES:");
				ImGui::Text(m_QuadEntity.GetComponents<TagComponent>().Tag.c_str());
			}

			ImGui::End();
		}

	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		

		return false;
	}


}

