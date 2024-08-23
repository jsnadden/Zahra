#pragma once

#include "Zahra.h"
#include "Zahra/Scene/Scene.h"

namespace Zahra
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context, const Ref<EditorCamera>& camera);

		void SetContext(const Ref<Scene>& context);
		void SetEditorCamera(const Ref<EditorCamera>& camera);

		void OnImGuiRender();

		// TODO: is there a better way of doing this (e.g. listeners have callback functions OnSelectionChange()?)
		Entity GetSelectedEntity() const { return m_Selected; }
		void SelectEntity(Entity entity) { m_Selected = entity; }


	private:
		Ref<Scene> m_Context;
		Ref<EditorCamera> m_Camera;

		Entity m_Selected;

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

		friend class PropertiesPanel;
	};

}


