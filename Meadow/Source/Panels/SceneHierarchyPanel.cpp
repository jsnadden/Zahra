#include "SceneHierarchyPanel.h"

#include "Editor/Editor.h"
#include "Utils/PanelUI.h"
#include "Zahra/Scripting/ScriptEngine.h"

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

	void SceneHierarchyPanel::OnImGuiRender(SceneState sceneState)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// HIERARCHY PANEL
		std::string windowName = "Scene Hierarchy: " + m_Context->GetName() + "###Scene Hierarchy";

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

		if (Editor::UnsavedChanges())
			flags |= ImGuiWindowFlags_UnsavedDocument;

		ImGui::Begin(windowName.c_str(), 0, flags);
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

						DrawEntityNode(entity, sceneState);

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
				DrawComponents(m_Selected, sceneState);

				// right click to add components
				if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
				{
					if (ImGui::MenuItem("Add Component(s)"))
						m_ShowAddComponentsModal = true;

					ImGui::EndPopup();
				}

				if (m_ShowAddComponentsModal)
					AddComponentsModal(m_Selected, sceneState);
			}
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, SceneState sceneState)
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

			if (ImGui::MenuItem("Delete entity", "", nullptr, sceneState == SceneState::Edit))
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
		
	void SceneHierarchyPanel::DrawComponents(Entity entity, SceneState sceneState)
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

		SceneHierarchyUIPatterns::DrawComponent<TransformComponent>("Transform Component", entity, [](auto& component)
				{
					SceneHierarchyUIPatterns::DrawFloat3Controls("Position", component.Translation);
					SceneHierarchyUIPatterns::DrawFloat3Controls("Dimensions", component.Scale, 1.0f, .05f, true);

					glm::vec3 rotation = glm::degrees(component.GetEulers());
					SceneHierarchyUIPatterns::DrawFloat3Controls("Euler Angles", rotation, .0f, 1.f);
					component.SetRotation(glm::radians(rotation));
				}, true, false);
		
		SceneHierarchyUIPatterns::DrawComponent<SpriteComponent>("Sprite Component", entity, [](auto& component)
			{
				SceneHierarchyUIPatterns::DrawRGBAControl("Tint Colour", component.Tint);
				SceneHierarchyUIPatterns::DrawTextureDrop("Sprite Sheet", component.Texture);
				SceneHierarchyUIPatterns::DrawFloatControl("Tiling Factor", component.TextureTiling, .01f, false, .0f, 100.f);
			});

		SceneHierarchyUIPatterns::DrawComponent<CircleComponent>("Circle Component", entity, [](auto& component)
			{
				SceneHierarchyUIPatterns::DrawRGBAControl("Colour", component.Colour);
				SceneHierarchyUIPatterns::DrawFloatControl("Thickness", component.Thickness, .01f, true, .01f, 1.f);
				SceneHierarchyUIPatterns::DrawFloatControl("Fade", component.Fade, .001f, true, .001f, 10.f, "%.3f");
			});

		SceneHierarchyUIPatterns::DrawComponent<ScriptComponent>("Script Component", entity, [&](auto& component)
			{
				// TODO: make this a combo box and populate from .first in ScriptEngine::GetScriptClasses
				// (also change ScriptComponent::ScriptName out for a script asset guid)

				bool validScript = ScriptEngine::ValidScriptClass(component.ScriptName);

				glm::vec3 textColour = glm::vec3(.95f, .1f, .1f);
				if (validScript)
					textColour = glm::vec3(.1f, .95f, .1f);

				char buffer[64];
				strcpy_s(buffer, component.ScriptName.c_str());
				
				if (ImGui::BeginTable(typeid(ScriptComponent).name(), 2))
				{
					ImGui::TableSetupColumn("labels", ImGuiTableColumnFlags_WidthFixed, 100.f);
					ImGui::TableSetupColumn("controls");

					if (SceneHierarchyUIPatterns::DrawTextEdit("Script Class", buffer, sizeof(buffer), textColour))
					{
						if (ImGui::IsWindowFocused())
							component.ScriptName = std::string(buffer);

						if (validScript = ScriptEngine::ValidScriptClass(component.ScriptName))
							m_Context->AllocateScriptFieldStorage(entity);
					}

					ImGui::EndTable();
				}

				if (validScript)
					SceneHierarchyUIPatterns::DrawScriptFieldTable(entity, sceneState, m_Context->GetScriptFieldStorage(entity));
								

			}, false, true, false);

		SceneHierarchyUIPatterns::DrawComponent<CameraComponent>("Camera Component", entity, [&](auto& component)
				{
					SceneCamera& camera = component.Camera;

					ImGui::TableNextColumn();
					ImGui::TableNextColumn();
					if (ImGui::Button("Make Active"))
					{
						m_Context->SetActiveCamera(entity);
					}

					const char* projectionTypeStrings[] = { "Orthographic", "Perspective" };
					SceneCamera::ProjectionType currentProjectionType = (SceneCamera::ProjectionType)SceneHierarchyUIPatterns::DrawComboControl("Projection Type", projectionTypeStrings, 2, (int)camera.GetProjectionType());
					camera.SetProjectionType(currentProjectionType);

					if (currentProjectionType == SceneCamera::ProjectionType::Orthographic)
					{
						float size = camera.GetOrthographicSize();
						SceneHierarchyUIPatterns::DrawFloatControl("Size", size, .05f, true, .5f, 50.f);
						camera.SetOrthographicSize(size);

						float nearClip = camera.GetOrthographicNearClip();
						SceneHierarchyUIPatterns::DrawFloatControl("Near-Clip Plane", nearClip, .01f);
						camera.SetOrthographicNearClip(nearClip);

						float farClip = camera.GetOrthographicFarClip();
						SceneHierarchyUIPatterns::DrawFloatControl("Far-Clip Plane", farClip, .01f);
						camera.SetOrthographicFarClip(farClip);
	
						SceneHierarchyUIPatterns::DrawBoolControl("Fixed Aspect Ratio", component.FixedAspectRatio);
					}

					if (currentProjectionType == SceneCamera::ProjectionType::Perspective)
					{
						float fov = glm::degrees(camera.GetPerspectiveFOV());
						SceneHierarchyUIPatterns::DrawFloatControl("FOV", fov, .1f, false, 10.f, 170.f);
						camera.SetPerspectiveFOV(glm::radians(fov));

						float nearClip = camera.GetPerspectiveNearClip();
						SceneHierarchyUIPatterns::DrawFloatControl("Near-Clip Plane", nearClip, .01f, false, .01f, 1.99f);
						camera.SetPerspectiveNearClip(nearClip);

						float farClip = camera.GetPerspectiveFarClip();
						SceneHierarchyUIPatterns::DrawFloatControl("Far-Clip Plane", farClip, 1.f, false, 2.f, 10000.f);
						camera.SetPerspectiveFarClip(farClip);
					}
				});
		
		SceneHierarchyUIPatterns::DrawComponent<RigidBody2DComponent>("2D Rigid Body Component", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				RigidBody2DComponent::BodyType currentBodyType = (RigidBody2DComponent::BodyType)SceneHierarchyUIPatterns::DrawComboControl("Body Type", bodyTypeStrings, 3, (int)component.Type);
				component.Type = currentBodyType;

				SceneHierarchyUIPatterns::DrawBoolControl("Non-Rotating", component.FixedRotation);
			});

		SceneHierarchyUIPatterns::DrawComponent<RectColliderComponent>("2D Rectangular Collider Component", entity, [](auto& component)
			{
				SceneHierarchyUIPatterns::DrawFloat2Controls("Offset", component.Offset);
				SceneHierarchyUIPatterns::DrawFloat2Controls("Size", component.HalfExtent, 0.5f, .05f, true);

				// TODO: investigate physically reasonable ranges
				SceneHierarchyUIPatterns::DrawFloatControl("Density", component.Density, .01f, false, .0f, 1.f);
				SceneHierarchyUIPatterns::DrawFloatControl("Friction", component.Friction, .01f, false, .0f, 1.f);
				SceneHierarchyUIPatterns::DrawFloatControl("Restitution", component.Restitution, .01f, false, .0f, 1.f);
				SceneHierarchyUIPatterns::DrawFloatControl("Rest. Threshold", component.RestitutionThreshold, .01f, false, .0f);
			});

		SceneHierarchyUIPatterns::DrawComponent<CircleColliderComponent>("2D Circular Collider Component", entity, [](auto& component)
			{
				SceneHierarchyUIPatterns::DrawFloat2Controls("Offset", component.Offset);
				SceneHierarchyUIPatterns::DrawFloatControl("Radius", component.Radius, .01f, true, .01f, 100.f);

				// TODO: investigate physically reasonable ranges
				SceneHierarchyUIPatterns::DrawFloatControl("Density", component.Density, .01f, false, .0f, 1.f);
				SceneHierarchyUIPatterns::DrawFloatControl("Friction", component.Friction, .01f, false, .0f, 1.f);
				SceneHierarchyUIPatterns::DrawFloatControl("Restitution", component.Restitution, .01f, false, .0f, 1.f);
				SceneHierarchyUIPatterns::DrawFloatControl("Rest. Threshold", component.RestitutionThreshold, .01f, false, .0f);
			});
		
	}
	
	void SceneHierarchyPanel::AddComponentsModal(Entity entity, SceneState sceneState)
	{
		// TODO: add checkboxes and an "add multiple" button

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
				clicked |= SceneHierarchyUIPatterns::AddComponentMenuItem<SpriteComponent>("Sprite", m_Selected);
				clicked |= SceneHierarchyUIPatterns::AddComponentMenuItem<CircleComponent>("Circle", m_Selected);
				clicked |= SceneHierarchyUIPatterns::AddComponentMenuItem<ScriptComponent>("Script", m_Selected);
				clicked |= SceneHierarchyUIPatterns::AddComponentMenuItem<CameraComponent>("Camera", m_Selected);
				clicked |= SceneHierarchyUIPatterns::AddComponentMenuItem<RigidBody2DComponent>("2D Rigid Body", m_Selected, sceneState == SceneState::Edit);
				clicked |= SceneHierarchyUIPatterns::AddComponentMenuItem<RectColliderComponent>("2D Rectangular Collider", m_Selected, sceneState == SceneState::Edit);
				clicked |= SceneHierarchyUIPatterns::AddComponentMenuItem<CircleColliderComponent>("2D Circular Collider", m_Selected, sceneState == SceneState::Edit);

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

