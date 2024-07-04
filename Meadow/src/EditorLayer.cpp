#include "EditorLayer.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>


namespace Zahra
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer"),
		m_CameraController(1280.0f / 720.0f, true)
	{
	}

	void EditorLayer::OnAttach()
	{
		m_Texture = Texture2D::Create("C:/dev/Zahra/Sandbox/assets/textures/yajirobe.png");

		FramebufferSpecification framebufferSpec;
		framebufferSpec.Width = 1280;
		framebufferSpec.Height = 720;// TODO: don't hardcode the size here!!

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
			Z_PROFILE_SCOPE("Camera update");

			m_CameraController.OnUpdate(dt);
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

				if (ImGui::BeginMenu("Edit"))
				{
					if (ImGui::MenuItem("???"));

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View"))
				{
					if (ImGui::MenuItem("Toggle Window Visibility (F1)")) m_ImguiWindowsVisible = !m_ImguiWindowsVisible;
					if (ImGui::MenuItem("Toggle Docking (F2)")) m_ImguiDockingEnabled = !m_ImguiDockingEnabled;

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Juwia"))
				{
					if (ImGui::BeginMenu("Hewwick"))
					{
						if (ImGui::MenuItem("That's wight bitcheees!!")) 0;

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}
			}
			ImGui::EndMainMenuBar();

		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// WINDOW DOCKSPACE
		if (m_ImguiDockingEnabled)
		{

			ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);

		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LEVEL VIEWER
		{
			ImGui::Begin("Level Viewer");
			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			Z_WARN("Viewer size: {0}x{1}", panelSize.x, panelSize.y);

			uint32_t framebufferID = m_Framebuffer->GetColourAttachmentRendererID();
			ImGui::Image((void*)framebufferID, ImVec2(1280.0f, 720.0f), ImVec2( 0, 1 ), ImVec2( 1, 0 ));
			
			ImGui::End();
		}

		if (m_ImguiWindowsVisible)
		{
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
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		if (Input::IsKeyPressed(Z_KEY_F1))
		{
			// toggle imgui window visibility
			m_ImguiWindowsVisible = !m_ImguiWindowsVisible;

			return true;
		}

		if (Input::IsKeyPressed(Z_KEY_F2))
		{
			// toggle imgui docking
			m_ImguiDockingEnabled = !m_ImguiDockingEnabled;

			return true;
		}

		return false;
	}


}

