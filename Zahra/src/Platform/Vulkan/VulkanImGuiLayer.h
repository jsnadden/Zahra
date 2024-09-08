#pragma once

#include "Zahra/ImGui/ImGuiLayer.h"

namespace Zahra
{
	class VulkanImGuiLayer : public ImGuiLayer
	{
	public:
		VulkanImGuiLayer();
		VulkanImGuiLayer(const std::string& name);
		virtual ~VulkanImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		virtual void Begin() override;
		virtual void End() override;

	};

}

