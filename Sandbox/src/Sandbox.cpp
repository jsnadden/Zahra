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
	
}

void SandboxLayer::OnDetach()
{
	
}

void SandboxLayer::OnUpdate(float dt)
{
	Zahra::Renderer::ResetStats();

	float aspectRatio = (float)Zahra::Renderer::GetSwapchainWidth() / (float)Zahra::Renderer::GetSwapchainHeight();

	// TODO: setup an editor camera instead of just setting these here
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	projection[1][1] *= -1.f; // TODO: rewrite things to avoid this stupid parity discrepancy

	Zahra::Renderer::BeginScene(view, projection);
	{
		Zahra::Renderer::ClearPass();

		int n = 20;
		float scale = 10.0f / n;

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				float x = -5.0f + (i + 0.5f) * scale;
				float y = -5.0f + (j + 0.5f) * scale;
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), { x, y, 0 }) * glm::scale(glm::mat4(1.0f), { scale, scale, scale });
				glm::vec4 colour = { .25f + .5f * ((float)i) / n, .25f + .5f * ((float)j) / n, .1f, 1.0f };
				Zahra::Renderer::DrawQuad(transform, colour);
			}
		}

	}
	Zahra::Renderer::EndScene();
}

void SandboxLayer::OnEvent(Zahra::Event& event)
{
	Zahra::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Zahra::KeyPressedEvent>(Z_BIND_EVENT_FN(SandboxLayer::OnKeyPressedEvent));
}

void SandboxLayer::OnImGuiRender()
{
	auto& allocationStats = Zahra::Memory::GetAllocationStats();
	auto& allocationStatsMap = Zahra::Allocator::GetAllocationStatsMap();
	auto& rendererStats = Zahra::Renderer::GetStats();

	if (ImGui::Begin("Stats", 0, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Text("Framerate: %.2f fps", Zahra::Application::Get().GetFramerate());
		
		ImGui::Separator();

		float currentAllocations = (float)(allocationStats.TotalAllocated - allocationStats.TotalFreed);
		if (currentAllocations >= BIT(20))
			ImGui::Text("Total allocations: %.1f mb", currentAllocations / BIT(20));
		else if (currentAllocations >= BIT(10))
			ImGui::Text("Total allocations: %.1f kb", currentAllocations / BIT(10));
		else
			ImGui::Text("Total allocations: %.1f b", currentAllocations);

		for (auto& [file, stats] : allocationStatsMap)
		{
			currentAllocations = (float)(stats.TotalAllocated - stats.TotalFreed);

			if (currentAllocations >= BIT(20))
				ImGui::Text("Total allocations from %s: %.1f mb", file, currentAllocations / BIT(20));
			else if (currentAllocations >= BIT(10))
				ImGui::Text("Total allocations from %s: %.1f kb", file, currentAllocations / BIT(10));
			else
				ImGui::Text("Total allocations from %s: %.1f b", file, currentAllocations);
		}

		/*ImGui::Text("Current allocated memory: %.2f mb", (float)(allocationStats.TotalAllocated - allocationStats.TotalFreed) / (float)(1<<20));
		ImGui::Text("Total allocated memory: %.2f mb", (float)allocationStats.TotalAllocated / (float)(1 << 20));*/
		
		ImGui::Separator();

		ImGui::Text("Draw calls per frame: %u", rendererStats.DrawCalls);

		ImGui::End();
	}
	
}

bool SandboxLayer::OnKeyPressedEvent(Zahra::KeyPressedEvent& event)
{
	return false;
}

