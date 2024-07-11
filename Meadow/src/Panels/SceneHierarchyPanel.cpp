#include "SceneHierarchyPanel.h"

#include <ImGui/imgui.h>


namespace Zahra
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		{
			m_Context->m_Registry.view<entt::entity>().each([&](auto entityId)
			{
				Entity entity { entityId, m_Context.get() };
				
				DrawEntityNode(entity);
				
			});
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponents<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0);
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		// TODO: find out how to detect clicking on the panel's empty space, so that I can set m_selected to a "null" value (i.e. deselect)
		if (ImGui::IsItemClicked())
		{
			m_Selected = entity;
		}

		if (opened)
		{
			ImGui::TreePop();
		}
	}

}

