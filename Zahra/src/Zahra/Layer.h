#pragma once

#include "Zahra/Base.h"
#include "Zahra/Events/Event.h"

namespace Zahra
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() { return m_DebugName; }

	protected:
		std::string m_DebugName;

	};
}

