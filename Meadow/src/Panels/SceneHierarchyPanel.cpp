#include "SceneHierarchyPanel.h"

#include "MeadowUIPatterns.h"

#include "Zahra/Scripting/ScriptEngine.h"

#include <iomanip>

namespace Zahra
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context, EditorCamera& camera)
	{
		SetContext(context);
		SetEditorCamera(camera);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Selected = {}; // can't reference an entity in a discarded scene!!
		m_Context = context;
	}

	void SceneHierarchyPanel::SetEditorCamera(EditorCamera& camera)
	{
		m_Camera = &camera;
	}

	void SceneHierarchyPanel::OnImGuiRender(bool physicsOn)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// HIERARCHY PANEL
		std::string windowName = "Scene Hierarchy: " + m_Context->GetName() + "###Scene Hierarchy";

		ImGui::Begin(windowName.c_str(), 0, ImGuiWindowFlags_NoCollapse);
		if (m_Context)
		{
			ImGui::BeginTable("SplitPanel", 2, ImGuiTableColumnFlags_NoResize);
			{
				ImGui::TableSetupColumn("EntityTree");
				ImGui::TableSetupColumn("Space", ImGuiTableColumnFlags_WidthFixed, 20.f);

				ImGui::TableNextColumn();

				// this forces the entities to be displayed in order of their entt::entity handle
				m_Context->m_Registry.sort<entt::entity>([](const auto& lhs, const auto& rhs) { return lhs < rhs; });
				
				// TODO: once we have parent/child relationships, this top
				// layer of the hierarchy should only include parentless entities
				auto entityView = m_Context->m_Registry.view<entt::entity>();
				entityView.use<entt::entity>();
				entityView.each([&](auto entityId)
					{
						Entity entity{ entityId, m_Context.Raw() };

						DrawEntityNode(entity, physicsOn);

					});

				ImGui::TableNextColumn();
			}
			ImGui::EndTable();

			// deselect entity when left clicking on empty window space (IsWindowHovered, without setting flags, is blocked by other items)
			if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
			{
				m_Selected = {};
			}

			// right clicking on empty window space brings up this menu
			if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Add New Entity"))
				{
					m_Selected = m_Context->CreateEntity("New Entity");
				}

				// TODO: menuitems to create specific scriptableentity types,
					// or entities with specific component sets
					// (e.g. a camera, an audio source, a prop/structure, or an npc)

				ImGui::EndPopup();
			}
		}
		ImGui::End();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// PROPERTIES PANEL
		ImGui::Begin("Properties", 0, ImGuiWindowFlags_NoCollapse);
		{
			if (m_Selected)
			{
				DrawComponents(m_Selected, physicsOn);

				// right click to add components
				if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
				{
					if (ImGui::MenuItem("Add Component(s)"))
						m_ShowAddComponentsModal = true;

					ImGui::EndPopup();
				}

				if (m_ShowAddComponentsModal)
					AddComponentsModal(m_Selected, physicsOn);
			}
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, bool physicsOn)
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

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				glm::vec3 center = entity.GetComponents<TransformComponent>().Translation;
				m_Camera->Recenter(center);
			}
		}

		bool entityDeleted = false;

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Add child", 0, false, false))
				int i = 0; // TODO: make this happen

			if (ImGui::MenuItem("Duplicate entity"))
				m_Selected = m_Context->DuplicateEntity(entity);

			if (ImGui::MenuItem("Delete entity", "", nullptr, !physicsOn))
				entityDeleted = true;

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
			if (m_Selected == entity)
				m_Selected = {};
		}

	}
		
	void SceneHierarchyPanel::DrawComponents(Entity entity, bool physicsOn)
	{ 
		Z_CORE_ASSERT(entity.HasComponents<TagComponent>(), "All entities must have a TagComponent")

			std::string& tag = entity.GetComponents<TagComponent>().Tag;

		// this necessitates a max tag size of 255 ascii characters (plus null terminator)
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, tag.c_str());

		if (ImGui::BeginTable("ID+Tag+Add", 2))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 50.0f);
			ImGui::TableSetupColumn("text");

			ImGui::TableNextColumn();
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Name");
			}

			ImGui::TableNextColumn();
			{
				ImGui::PushItemWidth(ImGui::GetColumnWidth());
				if (ImGui::InputText("", buffer, sizeof(buffer)))
				{
					if (ImGui::IsWindowFocused())
						tag = std::string(buffer);
				}
				ImGui::PopItemWidth();
			}

			ImGui::TableNextColumn();
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text("GUID");
			}

			ImGui::TableNextColumn();
			{
				std::stringstream stream;
				stream << "0x" << std::uppercase << std::hex << (uint64_t)entity.GetGUID();
				ImGui::Text(stream.str().c_str());

				ImGui::SameLine(ImGui::GetColumnWidth() - 135.f);
				
				if (ImGui::Button("Add Component(s)"))
					m_ShowAddComponentsModal = true;
				
			}

			ImGui::EndTable();
		}

		MeadowUIPatterns::DrawComponent<TransformComponent>("Transform Component", entity, [](auto& component)
				{
					MeadowUIPatterns::DrawFloat3Controls("Position", component.Translation);
					MeadowUIPatterns::DrawFloat3Controls("Dimensions", component.Scale, 1.0f, .05f, true);

					glm::vec3 rotation = glm::degrees(component.GetEulers());
					MeadowUIPatterns::DrawFloat3Controls("Euler Angles", rotation, .0f, 1.f);
					component.SetRotation(glm::radians(rotation));
				}, true, false);
		
		MeadowUIPatterns::DrawComponent<SpriteComponent>("Sprite Component", entity, [](auto& component)
			{
				MeadowUIPatterns::DrawRGBAControl("Tint Colour", component.Tint);
				MeadowUIPatterns::DrawTextureDrop("Sprite Sheet", component.Texture);
				MeadowUIPatterns::DrawFloatControl("Tiling Factor", component.TextureTiling, .01f, false, .0f, 100.f);
			});

		MeadowUIPatterns::DrawComponent<CircleComponent>("Circle Component", entity, [](auto& component)
			{
				MeadowUIPatterns::DrawRGBAControl("Colour", component.Colour);
				MeadowUIPatterns::DrawFloatControl("Thickness", component.Thickness, .01f, true, .01f, 1.f);
				MeadowUIPatterns::DrawFloatControl("Fade", component.Fade, .001f, true, .001f, 10.f, "%.3f");
			});

		MeadowUIPatterns::DrawComponent<ScriptComponent>("Script Component", entity, [entity](auto& component)
			{
				// TODO: make this a combo box and populate from .first in ScriptEngine::GetScriptClasses

				bool validScript = ScriptEngine::ValidScriptClass(component.ScriptName);

				glm::vec3 textColour = glm::vec3(.95f, .1f, .1f);
				if (validScript)
					textColour = glm::vec3(.1f, .95f, .1f);

				char buffer[64];
				strcpy_s(buffer, component.ScriptName.c_str());
				
				if (MeadowUIPatterns::DrawTextEdit("Script name", buffer, sizeof(buffer), textColour))
				{
					if (ImGui::IsWindowFocused())
						component.ScriptName = std::string(buffer);
				}

				Ref<ScriptInstance> instance;
				if (validScript)
					instance = ScriptEngine::GetScriptInstance(entity);

				if (!instance)
					return;

				for (auto& field : instance->GetEntityClass()->GetPublicFields())
				{
					switch (field.Type)
					{
						case ScriptFieldType::Float:
						{
							float value;
							instance->GetScriptFieldValue(field.Name, (void*)&value);

							if (MeadowUIPatterns::DrawFloatControl(field.Name, value))
								instance->SetScriptFieldValue(field.Name, (void*)&value);
						}
					}
				}

			});

		MeadowUIPatterns::DrawComponent<CameraComponent>("Camera Component", entity, [&](auto& component)
				{
					SceneCamera& camera = component.Camera;

					ImGui::TableNextColumn();
					ImGui::TableNextColumn();
					if (ImGui::Button("Make Active"))
					{
						m_Context->SetActiveCamera(entity);
					}



					const char* projectionTypeStrings[] = { "Orthographic", "Perspective" };
					SceneCamera::ProjectionType currentProjectionType = (SceneCamera::ProjectionType)MeadowUIPatterns::DrawComboControl("Projection Type", projectionTypeStrings, 2, (int)camera.GetProjectionType());
					camera.SetProjectionType(currentProjectionType);

					if (currentProjectionType == SceneCamera::ProjectionType::Orthographic)
					{
						float size = camera.GetOrthographicSize();
						MeadowUIPatterns::DrawFloatControl("Size", size, .05f, true, .5f, 50.f);
						camera.SetOrthographicSize(size);

						float nearClip = camera.GetOrthographicNearClip();
						MeadowUIPatterns::DrawFloatControl("Near-Clip Plane", nearClip, .01f);
						camera.SetOrthographicNearClip(nearClip);

						float farClip = camera.GetOrthographicFarClip();
						MeadowUIPatterns::DrawFloatControl("Far-Clip Plane", farClip, .01f);
						camera.SetOrthographicFarClip(farClip);
	
						MeadowUIPatterns::DrawBoolControl("Fixed Aspect Ratio", component.FixedAspectRatio);
					}

					if (currentProjectionType == SceneCamera::ProjectionType::Perspective)
					{
						float fov = glm::degrees(camera.GetPerspectiveFOV());
						MeadowUIPatterns::DrawFloatControl("FOV", fov, .1f, false, 10.f, 170.f);
						camera.SetPerspectiveFOV(glm::radians(fov));

						float nearClip = camera.GetPerspectiveNearClip();
						MeadowUIPatterns::DrawFloatControl("Near-Clip Plane", nearClip, .01f, false, .01f, 1.99f);
						camera.SetPerspectiveNearClip(nearClip);

						float farClip = camera.GetPerspectiveFarClip();
						MeadowUIPatterns::DrawFloatControl("Far-Clip Plane", farClip, 1.f, false, 2.f, 10000.f);
						camera.SetPerspectiveFarClip(farClip);
					}
				});
		
		MeadowUIPatterns::DrawComponent<RigidBody2DComponent>("2D Rigid Body Component", entity, [physicsOn](auto& component)
			{
				if (!physicsOn)
				{
					const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
					RigidBody2DComponent::BodyType currentBodyType = (RigidBody2DComponent::BodyType)MeadowUIPatterns::DrawComboControl("Body Type", bodyTypeStrings, 3, (int)component.Type);
					component.Type = currentBodyType;

					MeadowUIPatterns::DrawBoolControl("Non-Rotating", component.FixedRotation);
				}
			});

		MeadowUIPatterns::DrawComponent<RectColliderComponent>("2D Rectangular Collider Component", entity, [physicsOn](auto& component)
			{
				if (!physicsOn)
				{
					MeadowUIPatterns::DrawFloat2Controls("Offset", component.Offset);
					MeadowUIPatterns::DrawFloat2Controls("Size", component.HalfExtent, 0.5f, .05f, true);

					// TODO: investigate physically reasonable ranges
					MeadowUIPatterns::DrawFloatControl("Density", component.Density, .01f, false, .0f, 1.f);
					MeadowUIPatterns::DrawFloatControl("Friction", component.Friction, .01f, false, .0f, 1.f);
					MeadowUIPatterns::DrawFloatControl("Restitution", component.Restitution, .01f, false, .0f, 1.f);
					MeadowUIPatterns::DrawFloatControl("Rest. Threshold", component.RestitutionThreshold, .01f, false, .0f);
				}
			});

		MeadowUIPatterns::DrawComponent<CircleColliderComponent>("2D Circular Collider Component", entity, [physicsOn](auto& component)
			{
				if (!physicsOn)
				{
					MeadowUIPatterns::DrawFloat2Controls("Offset", component.Offset);
					MeadowUIPatterns::DrawFloatControl("Radius", component.Radius, .01f, true, .01f, 100.f);

					// TODO: investigate physically reasonable ranges
					MeadowUIPatterns::DrawFloatControl("Density", component.Density, .01f, false, .0f, 1.f);
					MeadowUIPatterns::DrawFloatControl("Friction", component.Friction, .01f, false, .0f, 1.f);
					MeadowUIPatterns::DrawFloatControl("Restitution", component.Restitution, .01f, false, .0f, 1.f);
					MeadowUIPatterns::DrawFloatControl("Rest. Threshold", component.RestitutionThreshold, .01f, false, .0f);
				}
			});
		
	}
	
	void SceneHierarchyPanel::AddComponentsModal(Entity entity, bool physicsOn)
	{
		ImGui::OpenPopup("Add Component(s)##Modal");

		ImVec2 center = ImGui::GetMousePos();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(.5f, .5f));

		if (ImGui::BeginPopupModal("Add Component(s)##Modal", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			bool clicked = false;

			ImGuiHoveredFlags hoveredFlags = ImGuiHoveredFlags_NoPopupHierarchy | ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows;
			clicked |= ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered(hoveredFlags);

			if (ImGui::BeginChild("Add Component(s)##ModalChild", ImVec2(200, 140)))
			{
				clicked |= MeadowUIPatterns::AddComponentMenuItem<SpriteComponent>("Sprite", m_Selected);
				clicked |= MeadowUIPatterns::AddComponentMenuItem<CircleComponent>("Circle", m_Selected);
				clicked |= MeadowUIPatterns::AddComponentMenuItem<ScriptComponent>("Script", m_Selected);
				clicked |= MeadowUIPatterns::AddComponentMenuItem<CameraComponent>("Camera", m_Selected);
				clicked |= MeadowUIPatterns::AddComponentMenuItem<RigidBody2DComponent>("2D Rigid Body", m_Selected, !physicsOn);
				clicked |= MeadowUIPatterns::AddComponentMenuItem<RectColliderComponent>("2D Rectangular Collider", m_Selected, !physicsOn);
				clicked |= MeadowUIPatterns::AddComponentMenuItem<CircleColliderComponent>("2D Circular Collider", m_Selected, !physicsOn);

				ImGui::EndChild();
			}

			clicked |= ImGui::Button("Close");
			

			if (clicked)
			{
				m_ShowAddComponentsModal = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

	}

	



}

