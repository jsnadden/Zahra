#include "Sandbox.h"

#include "Zahra/Utils/PlatformUtils.h"
#include "Zahra/Renderer/Shader.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Windows.h"

SandboxLayer::SandboxLayer()
	: Layer("Sandbox_Layer")
{
}

void SandboxLayer::OnAttach()
{
	m_FramerateRefreshTimer.Reset();

	Zahra::Renderer2DSpecification rendererSpec{};
	rendererSpec.RenderTarget = Zahra::Renderer::GetLoadPassFramebuffer();
	m_Renderer2D = Zahra::Ref<Zahra::Renderer2D>::Create(rendererSpec);

	m_Textures.resize(3);
	Zahra::Texture2DSpecification textureSpec{};
	m_Textures[0] = Zahra::Texture2D::CreateFromFile(textureSpec, "yajirobe.png");
	m_Textures[1] = Zahra::Texture2D::CreateFromFile(textureSpec, "checkerboard.png");
	m_Textures[2] = Zahra::Texture2D::CreateFromFile(textureSpec, "viking_room.png");
}

void SandboxLayer::OnDetach()
{
	m_Renderer2D.Reset();
}

void SandboxLayer::OnUpdate(float dt)
{
	m_Camera.OnUpdate(dt);

	if (m_FramerateRefreshTimer.Elapsed() >= c_FramerateRefreshInterval)
	{
		m_FramerateRefreshTimer.Reset();
		m_Framerate = 1.0f / dt;
	}

	float aspectRatio = (float)Zahra::Renderer::GetSwapchainWidth() / (float)Zahra::Renderer::GetSwapchainHeight();

	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	// TODO: setup an editor camera instead of just setting these here
	/*glm::mat4 view = glm::lookAt(glm::vec3(7.0f, 7.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::rotate(glm::mat4(1.0f), .5f * elapsedTime, { 0.0f, 0.0f, 1.0f });
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);*/
	glm::mat4 view = m_Camera.GetViewMatrix();
	glm::mat4 projection = m_Camera.GetProjection();
	projection[1][1] *= -1.f; // NOTE: remember to do this parity correction for all projections coming from glm

	//Zahra::Renderer::DrawTestScene(view, projection);

	m_Renderer2D->BeginScene(projection * view);
	{
		int n = 30;
		float scale = 10.0f / n;

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				float x = -5.0f + (i + 0.5f) * scale;
				float y = -5.0f + (j + 0.5f) * scale;

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), { y, .0f, x });
				transform *= glm::rotate(glm::mat4(1.0f), glm::atan(x) + elapsedTime, { 1.0f, 0.0f, 0.0f });
				transform *= glm::scale(glm::mat4(1.0f), { .8f * scale, .8f * scale, .8f * scale });

				glm::vec4 colour = { .25f + glm::cos(3 * elapsedTime) * .5f * ((float)i) / n, .25f + glm::sin(3 * elapsedTime) * .5f * ((float)j) / n, .1f, 1.0f};

				uint32_t texIndex = (i + j) % 3;

				m_Renderer2D->DrawQuad(transform, m_Textures[texIndex], colour);
				//m_Renderer2D->DrawQuadBoundingBox(transform, {1.0f, 1.0f, 0.0f, 1.0f});
				//m_Renderer2D->DrawCircle(transform, colour, .2f, .01f);
			}
		}

	}
	m_Renderer2D->EndScene();
}

void SandboxLayer::OnEvent(Zahra::Event& event)
{
	m_Camera.OnEvent(event);

	Zahra::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Zahra::KeyPressedEvent>(Z_BIND_EVENT_FN(SandboxLayer::OnKeyPressedEvent));
	dispatcher.Dispatch<Zahra::WindowResizedEvent>(Z_BIND_EVENT_FN(SandboxLayer::OnWindowResizedEvent));
}

void SandboxLayer::OnImGuiRender()
{
	auto& allocationStats = Zahra::Memory::GetAllocationStats();
	auto& allocationStatsMap = Zahra::Allocator::GetAllocationStatsMap();
	auto& renderer2DStats = m_Renderer2D->GetStats();

	if (ImGui::Begin("Engine Statistics", 0, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::SeparatorText("2D Batch Renderer");
		{
			ImGui::Text("Draw calls: %u", renderer2DStats.DrawCalls);
			ImGui::Text("Quads: %u", renderer2DStats.QuadCount);
			ImGui::Text("Quad batches: %u", renderer2DStats.QuadBatchCount);
			ImGui::Text("Circles: %u", renderer2DStats.CircleCount);
			ImGui::Text("Circle batches: %u", renderer2DStats.CircleBatchCount);
			ImGui::Text("Lines: %u", renderer2DStats.LineCount);
			ImGui::Text("Line batches: %u", renderer2DStats.LineBatchCount);

			/*float lineWidth = m_Renderer2D->GetLineWidth();
			ImGui::SliderFloat("Line width", &lineWidth, .1f, 10.f, "%.1f");
			m_Renderer2D->SetLineWidth(lineWidth);*/
		}	

		ImGui::SeparatorText("Timing");
		{
			ImGui::Text("Framerate: %.2f fps", m_Framerate);
		}

		ImGui::SeparatorText("Memory Allocations");
		{
			float currentAllocations;

			if (ImGui::BeginTable("MemoryUsageStats", 2, ImGuiTableColumnFlags_NoResize | ImGuiTableFlags_RowBg))
			{
				// TABLE SETUP
				{
					ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthStretch, 0);
					ImGui::TableSetupColumn("Usage", ImGuiTableColumnFlags_WidthFixed, 100);
					ImGui::TableHeadersRow();
				}

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

bool SandboxLayer::OnKeyPressedEvent(Zahra::KeyPressedEvent& event)
{
	return false;
}

bool SandboxLayer::OnWindowResizedEvent(Zahra::WindowResizedEvent& event)
{
	uint32_t width = event.GetWidth();
	uint32_t height = event.GetHeight();

	if (width == 0 || height == 0)
		return false;

	m_Renderer2D->OnWindowResize(width, height);
	m_Camera.SetViewportSize(width, height);

	return false;
}

