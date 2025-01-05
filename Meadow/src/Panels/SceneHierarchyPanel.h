#pragma once

#include "Zahra.h"
#include "Zahra/Scene/Scene.h"

namespace Zahra
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context, EditorCamera& camera);

		void SetContext(const Ref<Scene>& context);
		void SetEditorCamera(EditorCamera& camera);

		void OnImGuiRender(bool physicsOn);

		// TODO: is there a better way of doing this (e.g. listeners have callback functions OnSelectionChange()?)
		Entity GetSelectedEntity() const { return m_Selected; }
		void SelectEntity(Entity entity) { m_Selected = entity; }


	private:
		Ref<Scene> m_Context;
		EditorCamera* m_Camera = nullptr;

		Entity m_Selected;

		bool m_ShowAddComponentsModal = false;

		void DrawEntityNode(Entity entity, bool physicsOn);
		void DrawComponents(Entity entity, bool physicsOn);
		void AddComponentsModal(Entity entity, bool physicsOn);

		//friend class PropertiesPanel;
	};

}


