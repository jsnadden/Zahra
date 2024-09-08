#pragma once

#include "Zahra/Core/Defines.h"
#include "Zahra/Events/Event.h"
//#include "Zahra/Core/Timestep.h"

namespace Zahra
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float dt) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		const std::string& GetName() { return m_DebugName; }

	protected:
		std::string m_DebugName;

	};
}

