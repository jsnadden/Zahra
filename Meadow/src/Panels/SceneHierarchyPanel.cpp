#include "SceneHierarchyPanel.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

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

		// deselect when left clicking on empty window space (IsWindowHovered, without setting flags, is blocked by other items)
		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
		{
			m_Selected = {};
		}

		ImGui::End();

		ImGui::Begin("Properties");
		{
			if (m_Selected)
			{
				DrawComponents(m_Selected);
			}
		}
		ImGui::End();

	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		std::string& tag = entity.GetComponents<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0);
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{
			m_Selected = entity;
		}

		if (opened)
		{
			ImGui::TreePop();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponents<TagComponent>())
		{
			std::string& tag = entity.GetComponents<TagComponent>().Tag;

			// this necessitates a max tag size of 255 ascii characters (plus null terminator)
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, tag.c_str());

			if (ImGui::InputText("Entity name", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}

			if (entity.HasComponents<TransformComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf, "Transform"))
				{
					glm::mat4& transform = entity.GetComponents<TransformComponent>().Transform;

					//TODO: add rotation/resizing
					//TODO: adjust speed and min/max values
					ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), .05f, -100.f, 100.f);
				}
				ImGui::TreePop();

			}

			if (entity.HasComponents<CameraComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf, "Camera"))
				{
					auto& camera = entity.GetComponents<CameraComponent>().Camera;

					const char* projectionTypeStrings[] = { "Orthographic", "Perspective" };
					int currentProjectionType = (int)camera.GetProjectionType();

					if (ImGui::BeginCombo("Type", projectionTypeStrings[currentProjectionType]))
					{
						for (int i = 0; i < 2; i++)
						{
							if (ImGui::Selectable(projectionTypeStrings[i], currentProjectionType == i))
							{
								currentProjectionType = i;
								camera.SetProjectionType((SceneCamera::ProjectionType)i);
							}

							if (currentProjectionType == i)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
					{
						float size = camera.GetOrthographicSize();
						if (ImGui::DragFloat("Size", &size, .05, .5f, 20.0f, "%.2f", 32)) camera.SetOrthographicSize(size);

						float nearClip = camera.GetOrthographicNearClip();
						if (ImGui::DragFloat("Near", &nearClip, .01f)) camera.SetOrthographicNearClip(nearClip);

						float farClip = camera.GetOrthographicFarClip();
						if (ImGui::DragFloat("Far", &farClip, .01f)) camera.SetOrthographicFarClip(farClip);
					}

					if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
					{
						float fov = camera.GetPerspectiveFOV();
						if (ImGui::DragFloat("FOV", &fov, .01f, .9f, 3.1f)) camera.SetPerspectiveFOV(fov);

						float nearClip = camera.GetPerspectiveNearClip();
						if (ImGui::DragFloat("Near", &nearClip, .01f, .01f, 1.0f)) camera.SetPerspectiveNearClip(nearClip);

						float farClip = camera.GetPerspectiveFarClip();
						if (ImGui::DragFloat("Far", &farClip, 10.f, 10.f, 10000.f)) camera.SetPerspectiveFarClip(farClip);
					}

				}
				ImGui::TreePop();

			}

		}
	}

}

