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

	m_Framebuffer = Zahra::Renderer::GetLoadPassFramebuffer();

	m_Camera.SetViewportSize(m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight());

	Zahra::Renderer2DSpecification rendererSpec{};
	rendererSpec.RenderTarget = m_Framebuffer;
	m_Renderer2D = Zahra::Ref<Zahra::Renderer2D>::Create(rendererSpec);

	m_Scene = Zahra::Ref<Zahra::Scene>::Create();

	Zahra::Texture2DSpecification textureSpec{};
	m_Textures.emplace_back(Zahra::Texture2D::CreateFromFile(textureSpec, "yajirobe.png"));
	m_Textures.emplace_back(Zahra::Texture2D::CreateFromFile(textureSpec, "checkerboard.png"));

	{
		int n = 30;
		float scale = 10.0f / n;

		m_EntityGrid.resize(n);
		for (int i = 0; i < n; i++)
		{
			m_EntityGrid[i].resize(n);
			for (int j = 0; j < n; j++)
			{
				auto entity = m_Scene->CreateEntity(std::to_string(i) + "," + std::to_string(j));

				float x = -5.0f + (i + 0.5f) * scale;
				float y = -5.0f + (j + 0.5f) * scale;

				auto& tc = entity.GetComponents<Zahra::TransformComponent>();
				tc.Translation = { y, .0f, x };
				tc.EulerAngles = { glm::atan(x) , .0f, .0f };
				tc.Scale = { .8f * scale, .8f * scale, .8f * scale };

				auto& sc = entity.AddComponent<Zahra::SpriteComponent>();
				sc.Tint = { .25f + .5f * ((float)i) / n, .25f + .5f * ((float)j) / n, .1f, 1.0f };
				//sc.Texture = m_Textures[(i + j) % 2];

				m_EntityGrid[i][j] = entity;
			}
		}
	}
}

void SandboxLayer::OnDetach()
{
	m_EntityGrid.clear();
	m_Scene.Reset();
	m_Renderer2D.Reset();
}

void SandboxLayer::OnUpdate(float dt)
{
	m_Camera.OnUpdate(dt);

	//Zahra::Renderer::DrawTestScene(m_Camera.GetView(), m_Camera.GetProjection());

	if (m_FramerateRefreshTimer.Elapsed() >= c_FramerateRefreshInterval)
	{
		m_FramerateRefreshTimer.Reset();
		m_Framerate = 1.0f / dt;
	}

	uint32_t n = m_EntityGrid.size();
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			m_EntityGrid[i][j].GetComponents<Zahra::TransformComponent>().EulerAngles += glm::vec3(.0f, dt, .0f);
		}
	}

	m_Scene->OnRenderEditor(m_Renderer2D, m_Camera);
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

	m_Renderer2D->OnViewportResize(width, height);
	m_Camera.SetViewportSize(width, height);

	return false;
}

