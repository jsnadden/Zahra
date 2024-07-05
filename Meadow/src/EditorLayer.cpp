#include "EditorLayer.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>


namespace Zahra
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer"),
		m_CameraController(1280.0f / 720.0f)
	{
	}

	void EditorLayer::OnAttach()
	{
		m_Texture = Texture2D::Create("C:/dev/Zahra/Sandbox/assets/textures/yajirobe.png");

		FramebufferSpecification framebufferSpec;
		framebufferSpec.Width = 1280;
		framebufferSpec.Height = 720; // These will be overwritten by the ImGui viewport window

		m_Framebuffer = Framebuffer::Create(framebufferSpec);
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

			Zahra::FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f // framebuffer requires positive dimensions
				&& (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
			{
				m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				m_CameraController.Resize(m_ViewportSize.x, m_ViewportSize.y);
			}
		}

		{
			Z_PROFILE_SCOPE("Camera update");

			if (m_ViewportFocused) m_CameraController.OnUpdate(dt);
		}

		Renderer2D::ResetStats();

		{
			Z_PROFILE_SCOPE("Renderer clear");

			m_Framebuffer->Bind();

			RenderCommand::SetClearColour(glm::make_vec4(m_ClearColour));
			RenderCommand::Clear();
		}

		{
			Z_PROFILE_SCOPE("Renderer draw");

			Renderer2D::BeginScene(m_CameraController.GetCamera());

			glm::vec2 dims = glm::make_vec2(m_QuadDimensions);
			glm::vec4 tint = glm::make_vec4(m_QuadColour);
			glm::vec2 pos = glm::make_vec2(m_QuadPosition);

			int n = 20;

			Renderer2D::DrawQuad({ 0.0f, 0.0f, -.1f }, 9.0f * dims, m_Texture);

			for (int i = 0; i < n * n; i++)
			{
				glm::vec2 gridpoint(i % n, glm::floor((float)i / n));

				Renderer2D::DrawRotatedQuad(
					pos + (10.0f / (float)n) * (gridpoint - glm::vec2(((float)n - 1) / 2,				// POSITION
						((float)n - 1) / 2)) * dims, dims * glm::vec2(10.0f / (float)n, 10.0f / (float)n),  // DIMENSIONS
					m_QuadRotation,																		// ROTATION
					tint * glm::vec4((1 / (float)n) * gridpoint, .4f, .8f)								// COLOUR
				);
			}

			Renderer2D::EndScene();
			m_Framebuffer->Unbind();
		}

	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_CameraController.OnEvent(event);

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
			ImGui::Begin("Rendering Demo Controls");

			ImGui::ColorEdit3("Background colour", m_ClearColour);

			ImGui::SliderFloat2("Quad position", m_QuadPosition, -5.0f, 5.0f);
			ImGui::SliderFloat2("Quad dimensions", m_QuadDimensions, .01f, 10.0f, "%.3f", 32);
			ImGui::SliderFloat("Quad rotation", &m_QuadRotation, -3.14f, 3.14f);
			ImGui::ColorEdit4("Quad tint", m_QuadColour);

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

