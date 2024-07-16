#include "SceneHierarchyPanel.h"

#include "StylePatterns.h"

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
		ImGui::Begin("Scene Hierarchy", 0, ImGuiWindowFlags_NoCollapse);
		{
			// TODO: this top layer of the hierarchy should only include parentless entities
			m_Context->m_Registry.view<entt::entity>().each([&](auto entityId)
			{
				Entity entity { entityId, m_Context.get() };
				
				DrawEntityNode(entity);
				
			});
		}
		
		// deselect entity when left clicking on empty window space (IsWindowHovered, without setting flags, is blocked by other items)
		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
		{
			m_Selected = {};
		}

		// right clicking on empty window space brings up this menu
		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create New Entity"))
			{
				m_Selected = m_Context->CreateEntity("New Entity");
			}

			// TODO: menuitems to create specific scriptableentity types,
				// or entities with specific component sets
				// (e.g. a camera, an audio source, a prop/structure, or an npc)

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties", 0, ImGuiWindowFlags_NoCollapse);
		{
			if (m_Selected)
			{
				DrawComponents(m_Selected);

				if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
				{
					// TODO: keep adding to this list as we include new component types (e.g. scripts!!)

					if (ImGui::MenuItem("Camera", 0, false, !m_Selected.HasComponents<CameraComponent>()))
					{
						m_Selected.AddComponent<CameraComponent>();
						ImGui::CloseCurrentPopup();
					}

					if (ImGui::MenuItem("Sprite", 0, false, !m_Selected.HasComponents<SpriteComponent>()))
					{
						m_Selected.AddComponent<SpriteComponent>();
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		std::string& tag = entity.GetComponents<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			( m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0 );

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{
			m_Selected = entity;
		}

		bool entityDeleted = false;

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Add child", 0, false, false)); // TODO: make this happen
			if (ImGui::MenuItem("Delete entity")) entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			// TODO: call this same method recursively on children

			ImGui::TreePop();
		}

		// cleanup
		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_Selected == entity) m_Selected = {};
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

			if (ImGui::BeginTable("entitytag", 2))
			{
				ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("text");

				ImGui::TableNextColumn();				
				{
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Name");
				}
				
				ImGui::TableNextColumn();
				{
					ImGui::PushItemWidth(ImGui::GetColumnWidth());
					if (ImGui::InputText("", buffer, sizeof(buffer))) tag = std::string(buffer);
					ImGui::PopItemWidth();
				}

				ImGui::EndTable();
			}			
		}

		StylePatterns::DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
				{
					StylePatterns::DrawVec3Controls("Position", component.Translation);
					StylePatterns::DrawVec3Controls("Dimensions", component.Scale, 1.0f, .05f, true);

					glm::vec3& rotation = glm::degrees(component.EulerAngles);
					StylePatterns::DrawVec3Controls("Euler Angles", rotation, .0f, 1.f);
					component.EulerAngles = glm::radians(rotation);					
				});
		
		StylePatterns::DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
				{
					SceneCamera& camera = component.Camera;

					ImGui::Checkbox("Active", &component.Active);

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
						if (ImGui::DragFloat("Size", &size, .05f, .5f, 50.0f, "%.2f", 32)) camera.SetOrthographicSize(size);

						float nearClip = camera.GetOrthographicNearClip();
						if (ImGui::DragFloat("Near", &nearClip, .01f)) camera.SetOrthographicNearClip(nearClip);

						float farClip = camera.GetOrthographicFarClip();
						if (ImGui::DragFloat("Far", &farClip, .01f)) camera.SetOrthographicFarClip(farClip);


						ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
					}

					if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
					{
						float fov = glm::degrees(camera.GetPerspectiveFOV());
						if (ImGui::DragFloat("FOV", &fov, .1f, 10.f, 170.f, "%.1f")) camera.SetPerspectiveFOV(glm::radians(fov));

						float nearClip = camera.GetPerspectiveNearClip();
						if (ImGui::DragFloat("Near", &nearClip, .01f, .01f, 1.99f, "%.2f")) camera.SetPerspectiveNearClip(nearClip);

						float farClip = camera.GetPerspectiveFarClip();
						if (ImGui::DragFloat("Far", &farClip, 1.f, 2.f, 10000.f, "%.0f")) camera.SetPerspectiveFarClip(farClip);
					}
				});
		
		StylePatterns::DrawComponent<SpriteComponent>("Sprite", entity, [](auto& component)
				{
					ImGui::ColorEdit4("Colour", glm::value_ptr(component.Colour));
				});
		
	}

	



}

