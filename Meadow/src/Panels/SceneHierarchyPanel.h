#pragma once

#include "Zahra.h"
#include "Zahra/Scene/Scene.h"

namespace Zahra
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);

		void OnImGuiRender();

	private:
		Ref<Scene> m_Context;

		Entity m_Selected;

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

		friend class PropertiesPanel;
	};

}


