#include "Sandbox.h"

#include "Zahra/Utils/PlatformUtils.h"
#include "Zahra/Renderer/Shader.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include "Windows.h"

SandboxLayer::SandboxLayer()
	: Layer("Sandbox_Layer")
{
}

void SandboxLayer::OnAttach()
{
	m_FramerateRefreshTimer.Reset();

	{
		Zahra::Image2DSpecification imageSpec{};
		imageSpec.Name = "Sandbox_ViewportImage";
		imageSpec.Format = Zahra::ImageFormat::RGBA_UN;
		imageSpec.Width = m_ViewportWidth;
		imageSpec.Height = m_ViewportHeight;
		imageSpec.Sampled = true;
		m_ViewportRenderTarget = Zahra::Image2D::Create(imageSpec);

		m_ViewportTexture = Zahra::Texture2D::CreateFromImage2D(m_ViewportRenderTarget);
		m_ViewportTextureHandle = Zahra::Application::Get().GetImGuiLayer()->RegisterTexture(m_ViewportTexture);

		Zahra::FramebufferSpecification framebufferSpec{};
		framebufferSpec.Name = "Sandbox_ViewportFramebuffer";
		framebufferSpec.Width = m_ViewportWidth;
		framebufferSpec.Height = m_ViewportHeight;
		{
			auto& attachment = framebufferSpec.ColourAttachmentSpecs.emplace_back();
			attachment.InheritFrom = m_ViewportRenderTarget;
			attachment.Format = Zahra::ImageFormat::RGBA_UN;
		}
		framebufferSpec.HasDepthStencil = true;
		framebufferSpec.DepthClearValue = 1.0f;
		framebufferSpec.DepthStencilAttachmentSpec.Format = Zahra::ImageFormat::DepthStencil;
		m_ViewportFramebuffer = Zahra::Framebuffer::Create(framebufferSpec);
	}

	Zahra::RenderPassSpecification renderPassSpec{};
	renderPassSpec.Name = "Sandbox_ClearPass";
	renderPassSpec.RenderTarget = m_ViewportFramebuffer;
	renderPassSpec.ClearColourAttachments = true;
	renderPassSpec.ClearDepthAttachment = true;
	m_ClearPass = Zahra::RenderPass::Create(renderPassSpec);

	Zahra::Renderer2DSpecification rendererSpec{};
	rendererSpec.RenderTarget = m_ViewportFramebuffer;
	m_Renderer2D = Zahra::Ref<Zahra::Renderer2D>::Create(rendererSpec);

	m_Scene = Zahra::Ref<Zahra::Scene>::Create();

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
			tc.SetRotation({ glm::atan(x) , .0f, .0f });
			tc.Scale = { .8f * scale, .8f * scale, .8f * scale };

			auto& sc = entity.AddComponent<Zahra::SpriteComponent>();
			sc.Tint = { .25f + .5f * ((float)i) / n, .25f + .5f * ((float)j) / n, .1f, 1.0f };

			m_EntityGrid[i][j] = entity;
		}
	}

	//OnViewportResize();
}

void SandboxLayer::OnDetach()
{
	m_EntityGrid.clear();

	m_Renderer2D.Reset();
	m_Scene.Reset();

	Zahra::Application::Get().GetImGuiLayer()->DeregisterTexture(m_ViewportTextureHandle);
	m_ViewportTexture.Reset();

	m_ClearPass.Reset();
	m_ViewportFramebuffer.Reset();
	m_ViewportRenderTarget.Reset();
}

void SandboxLayer::OnUpdate(float dt)
{
	m_Resized = false;

	if (m_ViewportRenderTarget->GetWidth() != m_ViewportWidth || m_ViewportRenderTarget->GetHeight() != m_ViewportHeight)
	{
		OnViewportResize();
		m_Resized = true;
	}

	m_Camera.OnUpdate(dt);

	if (m_FramerateRefreshTimer.Elapsed() >= c_FramerateRefreshInterval)
	{
		m_FramerateRefreshTimer.Reset();
		m_Framerate = 1.0f / dt;
	}

	uint32_t n = (uint32_t)m_EntityGrid.size();
	for (uint32_t i = 0; i < n; i++)
	{
		for (uint32_t j = 0; j < n; j++)
		{
			auto& tc = m_EntityGrid[i][j].GetComponents<Zahra::TransformComponent>();
			glm::vec3 eulers = tc.GetEulers();
			eulers += glm::vec3(.0f, dt, .0f);
			tc.SetRotation(eulers);
		}
	}

	// TEMPORARY (clear viewport framebuffer)
	Zahra::Renderer::BeginRenderPass(m_ClearPass, false, true);
	Zahra::Renderer::EndRenderPass();

	m_Scene->OnRenderEditor(m_Renderer2D, m_Camera, {}, {});
}

void SandboxLayer::OnImGuiRender()
{
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton);

	if (ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::GetWindowViewport()->Flags;

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportWidth = (uint32_t)viewportPanelSize.x;
		m_ViewportHeight = (uint32_t)viewportPanelSize.y;

		Zahra::Application::Get().GetImGuiLayer()->BlockEvents(false);

		if (!m_Resized)
			ImGui::Image(m_ViewportTextureHandle, ImVec2((float)m_ViewportWidth, (float)m_ViewportHeight), ImVec2(0, 0), ImVec2(1, 1));

		ImGui::End();
	}

	auto& allocationStats = Zahra::Memory::GetAllocationStats();
	auto& allocationStatsMap = Zahra::Allocator::GetAllocationStatsMap();
	auto& renderer2DStats = m_Renderer2D->GetStats();

	if (ImGui::Begin("Engine Statistics", 0, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::SeparatorText("Timing");
		{
			ImGui::Text("Framerate: %.2f fps", m_Framerate);
		}

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

void SandboxLayer::OnViewportResize()
{
	Zahra::Application::Get().GetImGuiLayer()->DeregisterTexture(m_ViewportTextureHandle);
	m_ViewportRenderTarget->Resize(m_ViewportWidth, m_ViewportHeight);
	m_ViewportFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
	m_ClearPass->OnResize();

	m_ViewportTexture->Resize(m_ViewportWidth, m_ViewportHeight);
	m_ViewportTextureHandle = Zahra::Application::Get().GetImGuiLayer()->RegisterTexture(m_ViewportTexture);

	m_Camera.SetViewportSize((float)m_ViewportWidth, (float)m_ViewportHeight);

	m_Renderer2D->OnViewportResize(m_ViewportWidth, m_ViewportHeight);
}

void SandboxLayer::OnEvent(Zahra::Event& event)
{
	m_Camera.OnEvent(event);

	Zahra::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Zahra::KeyPressedEvent>(Z_BIND_EVENT_FN(SandboxLayer::OnKeyPressedEvent));
	dispatcher.Dispatch<Zahra::WindowResizedEvent>(Z_BIND_EVENT_FN(SandboxLayer::OnWindowResizedEvent));
}

bool SandboxLayer::OnKeyPressedEvent(Zahra::KeyPressedEvent& event)
{
	return false;
}

bool SandboxLayer::OnWindowResizedEvent(Zahra::WindowResizedEvent& event)
{
	return false;
}

