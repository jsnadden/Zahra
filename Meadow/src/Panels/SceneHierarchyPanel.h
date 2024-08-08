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

		void OnImGuiRender(bool editing);

		// TODO: find a better way of doing this (e.g. listeners have callback functions OnSelectionChange())
		Entity GetSelectedEntity() const { return m_Selected; }
		void SelectEntity(Entity entity) { m_Selected = entity; }

	private:
		Ref<Scene> m_Context;

		Entity m_Selected;

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

		friend class PropertiesPanel;
	};

}


