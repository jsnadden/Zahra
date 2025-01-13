#pragma once

#include "Utils/TypeDefs.h"

#include <Zahra.h>

namespace Zahra
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context, EditorCamera& camera);

		void SetContext(const Ref<Scene>& context);
		void SetEditorCamera(EditorCamera& camera);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_Selected; }
		void SelectEntity(Entity entity) { m_Selected = entity; }
		bool IsSelected(ZGUID entityID);
		void Deselect() { m_Selected = {}; }


	private:
		WeakRef<Scene> m_Context;
		WeakRef<EditorCamera> m_Camera;

		Entity m_Selected;

		bool m_ShowAddComponentsModal = false;

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		void AddComponentsModal(Entity entity);

	};

}


