#include "SceneHierarchyPanel.h"

#include "Editor/Editor.h"
#include "UI/Elements/ComponentUI.h"

#include "Zahra/Scripting/ScriptEngine.h"

namespace Zahra
{
	SceneHierarchyPanel::SceneHierarchyPanel()
	{
		ScriptEngine::AddReloadCallback([&]() { CacheScriptClassNames(); });
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// HIERARCHY PANEL
		auto scene = Editor::GetSceneContext();
		std::string windowName = "Scene Hierarchy: " + scene->GetName() + "###Scene Hierarchy";

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

		/*if (Editor::UnsavedChanges())
			flags |= ImGuiWindowFlags_UnsavedDocument;*/

		ImGui::Begin(windowName.c_str(), 0, flags);
		if (scene)
		{
			ImGui::BeginTable("SplitPanel", 2, ImGuiTableColumnFlags_NoResize);
			{
				ImGui::TableSetupColumn("EntityTree");
				ImGui::TableSetupColumn("Space", ImGuiTableColumnFlags_WidthFixed, 20.f);

				ImGui::TableNextColumn();
				
				// TODO: once we have parent/child relationships, this top
				// layer of the hierarchy should only include parentless entities
				scene->ForEachEntity([&](Entity entity) { DrawEntityNode(entity); });

				ImGui::TableNextColumn();
			}
			ImGui::EndTable();

			// deselect entity when left clicking on empty window space (IsWindowHovered, without setting flags, is blocked by other items)
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			{
				Editor::SelectEntity({});
			}

			// right clicking on empty window space brings up this menu
			if (ImGui::BeginPopupContextWindow("##SceneHierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Add New Entity"))
					scene->CreateEntity();

				// TODO: menuitems to create prefab entities (with specific component sets and default parameters)

				ImGui::EndPopup();
			}
		}
		ImGui::End();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// PROPERTIES PANEL
		ImGui::Begin("Properties", 0, ImGuiWindowFlags_NoCollapse);
		{
			Entity selected = Editor::GetSelectedEntity();
			if (selected)
			{
				DrawComponents(selected);

				// right click to add components
				if (ImGui::BeginPopupContextWindow("##PropertiesContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
				{
					if (ImGui::MenuItem("Add Component(s)"))
						m_ShowAddComponentsModal = true;

					ImGui::EndPopup();
				}

				if (m_ShowAddComponentsModal)
					AddComponentsModal(selected);
			}
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::CacheScriptClassNames()
	{
		m_ScriptClassNames.clear();

		// placeholder string for invalid script names
		m_ScriptClassNames.emplace_back("-");

		auto& scriptClasses = ScriptEngine::GetScriptClasses();
		m_ScriptClassCount = scriptClasses.size();

		for (auto& [name, scriptClass] : scriptClasses)
		{
			m_ScriptClassNames.emplace_back(name.c_str());
		}

		// TODO: sort vector by script use frequency: occasionally poll the
		// scene's ECS for a histogram of valid attached script classes
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto scene = Editor::GetSceneContext();

		std::string& tag = entity.GetComponents<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			(Editor::GetSelectedEntity() == entity ? ImGuiTreeNodeFlags_Selected : 0 );

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{
			Editor::SelectEntity(entity);

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				glm::vec3 center = entity.GetComponents<TransformComponent>().Translation;
				Editor::CenterPrimaryEditorCamera(center);
			}
		}

		bool entityToTheGallows = false;

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Add child", nullptr, false, Editor::GetSceneState() == SceneState::Edit))
			{
				// TODO: implement this with HierarchyComponent
			}

			if (ImGui::MenuItem("Duplicate entity", nullptr, false, Editor::GetSceneState() == SceneState::Edit))
			{
				if (entity)
					scene->DuplicateEntity(entity);
			}

			if (ImGui::MenuItem("Delete entity", nullptr, false, Editor::GetSceneState() == SceneState::Edit))
				entityToTheGallows = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			// TODO: call this same method recursively on children

			ImGui::TreePop();
		}

		if (entityToTheGallows)
		{
			// TODO: replace with an EditAction submission
			if (Editor::GetSelectedEntity() == entity)
				Editor::SelectEntity({});

			scene->DestroyEntity(entity);
		}
	}
		
	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{ 
		auto scene = Editor::GetSceneContext();

		Z_CORE_ASSERT(entity.HasComponents<TagComponent>(), "All entities must have a TagComponent")

		std::string& tag = entity.GetComponents<TagComponent>().Tag;
		UUID entityID = entity.GetID();

		ImGui::PushID((int)entityID);

		// TODO: define a global preprocessor constant for maximum entity name length
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), tag.c_str());

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
				ImGui::Text("UUID");
			}

			ImGui::TableNextColumn();
			{
				std::stringstream stream;
				stream << "0x" << std::uppercase << std::hex << (uint64_t)entityID;
				ImGui::Text(stream.str().c_str());

				ImGui::SameLine(ImGui::GetColumnWidth() - 135.f);
				
				if (ImGui::Button("Add Component(s)"))
					m_ShowAddComponentsModal = true;
				
			}

			ImGui::EndTable();
		}

		ComponentUI::DrawComponent<TransformComponent>("Transform Component", entity, [](auto& component)
				{
					ComponentUI::DrawFloat3Controls("Position", component.Translation);
					ComponentUI::DrawFloat3Controls("Dimensions", component.Scale, 1.0f, .05f, true);
					ComponentUI::DrawEulerAngleControls(component);
				}, true, false);
		
		ComponentUI::DrawComponent<SpriteComponent>("Sprite Component", entity, [](auto& component)
			{
				ComponentUI::DrawRGBAControl("Tint Colour", component.Tint);
				ComponentUI::DrawTexturePreview("Texture", component.TextureHandle);
				ComponentUI::DrawFloatControl("Tiling Factor", component.TextureTiling, .01f, false, .0f, 100.f);
			});

		ComponentUI::DrawComponent<CircleComponent>("Circle Component", entity, [](auto& component)
			{
				ComponentUI::DrawRGBAControl("Colour", component.Colour);
				ComponentUI::DrawFloatControl("Thickness", component.Thickness, .01f, true, .01f, 1.f);
				ComponentUI::DrawFloatControl("Fade", component.Fade, .001f, true, .001f, 10.f, "%.3f");
			});

		ComponentUI::DrawComponent<ScriptComponent>("Script Component", entity, [&](auto& component)
			{
				// TODO: very hacky, and not particularly informative, but it'll do for now
				if (!ScriptEngine::AppAssemblyAlreadyLoaded())
					return;

				uint32_t currentIndex = 0;

				// TODO: this lookup spits on the grave of Alan Turing et al.:
					//  - Sort the vector by script instance count (see note in CacheScriptClassNames())
					//  - Once I have an AssetManager I can search by AssetID instead of slow-ass strings
				for (uint32_t index = 0; index <= m_ScriptClassCount; index++)
				{
					if (m_ScriptClassNames[index] == component.ScriptName)
					{
						currentIndex = index;
						break;
					}
				}

				const char* label = "Script Class";

				if (ImGui::BeginTable(typeid(ScriptComponent).name(), 2))
				{
					ImGui::TableSetupColumn("labels", ImGuiTableColumnFlags_WidthFixed, 100.f);
					ImGui::TableSetupColumn("controls");

					if (Editor::GetSceneState() == SceneState::Play)
					{
						ImGui::PushID(label);
						ImGui::TableNextColumn();
						{
							ImGui::AlignTextToFramePadding();
							ImGui::Text(label);
						}
						ImGui::TableNextColumn();
						{
							ImGui::AlignTextToFramePadding();
							ImGui::Text(m_ScriptClassNames[currentIndex]);
						}
						ImGui::PopID();
					}
					else
					{
						if (ComponentUI::DrawComboControl(label, m_ScriptClassNames, currentIndex))
						{
							component.ScriptName = m_ScriptClassNames[currentIndex];

							Z_CORE_ASSERT(ScriptEngine::ValidScriptClass(component.ScriptName) || currentIndex == 0);

							if (currentIndex)
								scene->AllocateScriptFieldStorage(entity);
						}
					}
					

					ImGui::EndTable();
				}
				
				// at this stage, currentIndex should be zero if and only if component.ScriptName is invalid
				if (currentIndex)
					ComponentUI::DrawScriptFieldTable(entity, scene->GetScriptFieldStorage(entity));
								

			}, false, true, false);

		ComponentUI::DrawComponent<CameraComponent>("Camera Component", entity, [&](auto& component)
				{
					SceneCamera& camera = component.Camera;

					ImGui::TableNextColumn();
					ImGui::TableNextColumn();

					bool isActiveCamera = scene->GetActiveCamera() == entity;
					if (isActiveCamera)
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
					}
					if (ImGui::Button("Make Active"))
					{
						scene->SetActiveCamera(entity);
					}
					if (isActiveCamera)
					{
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();
					}					

					std::vector<const char*> projectionTypeStrings = { "Orthographic", "Perspective" };
					auto comboIndex = (uint32_t)camera.GetProjectionType();

					ComponentUI::DrawComboControl("Projection Type", projectionTypeStrings, comboIndex);

					auto currentProjectionType = (SceneCamera::ProjectionType)comboIndex;
					camera.SetProjectionType(currentProjectionType);

					if (currentProjectionType == SceneCamera::ProjectionType::Orthographic)
					{
						float size = camera.GetOrthographicSize();
						ComponentUI::DrawFloatControl("Size", size, .05f, true, .5f, 50.f);
						camera.SetOrthographicSize(size);

						float nearClip = camera.GetOrthographicNearClip();
						ComponentUI::DrawFloatControl("Near-Clip Plane", nearClip, .01f);
						camera.SetOrthographicNearClip(nearClip);

						float farClip = camera.GetOrthographicFarClip();
						ComponentUI::DrawFloatControl("Far-Clip Plane", farClip, .01f);
						camera.SetOrthographicFarClip(farClip);
	
						ComponentUI::DrawBoolControl("Fixed Aspect Ratio", component.FixedAspectRatio);
					}

					if (currentProjectionType == SceneCamera::ProjectionType::Perspective)
					{
						float fov = glm::degrees(camera.GetPerspectiveFOV());
						ComponentUI::DrawFloatControl("FOV", fov, .1f, false, 10.f, 170.f);
						camera.SetPerspectiveFOV(glm::radians(fov));

						float nearClip = camera.GetPerspectiveNearClip();
						ComponentUI::DrawFloatControl("Near-Clip Plane", nearClip, .01f, false, .01f, 1.99f);
						camera.SetPerspectiveNearClip(nearClip);

						float farClip = camera.GetPerspectiveFarClip();
						ComponentUI::DrawFloatControl("Far-Clip Plane", farClip, 1.f, false, 2.f, 10000.f);
						camera.SetPerspectiveFarClip(farClip);
					}
				});
		
		ComponentUI::DrawComponent<RigidBody2DComponent>("2D Rigid Body Component", entity, [](auto& component)
			{
				std::vector<const char*> bodyTypeStrings = { "Static", "Dynamic", "Kinematic" };
				auto comboIndex = (uint32_t)component.Type;

				ComponentUI::DrawComboControl("Body Type", bodyTypeStrings, comboIndex, Editor::GetSceneState() != SceneState::Edit);

				component.Type = (RigidBody2DComponent::BodyType)comboIndex;

				ComponentUI::DrawBoolControl("Non-Rotating", component.FixedRotation, Editor::GetSceneState() != SceneState::Edit);
			});

		ComponentUI::DrawComponent<RectColliderComponent>("2D Rectangular Collider Component", entity, [](auto& component)
			{
				ComponentUI::DrawFloat2Controls("Offset", component.Offset);
				ComponentUI::DrawFloat2Controls("Size", component.HalfExtent, 0.5f, .05f, true);

				// TODO: investigate physically reasonable ranges
				ComponentUI::DrawFloatControl("Density", component.Density, .01f, false, .0f, 1.f);
				ComponentUI::DrawFloatControl("Friction", component.Friction, .01f, false, .0f, 1.f);
				ComponentUI::DrawFloatControl("Restitution", component.Restitution, .01f, false, .0f, 1.f);
				ComponentUI::DrawFloatControl("Rest. Threshold", component.RestitutionThreshold, .01f, false, .0f);
			});

		ComponentUI::DrawComponent<CircleColliderComponent>("2D Circular Collider Component", entity, [](auto& component)
			{
				ComponentUI::DrawFloat2Controls("Offset", component.Offset);
				ComponentUI::DrawFloatControl("Radius", component.Radius, .01f, true, .01f, 100.f);

				// TODO: investigate physically reasonable ranges
				ComponentUI::DrawFloatControl("Density", component.Density, .01f, false, .0f, 1.f);
				ComponentUI::DrawFloatControl("Friction", component.Friction, .01f, false, .0f, 1.f);
				ComponentUI::DrawFloatControl("Restitution", component.Restitution, .01f, false, .0f, 1.f);
				ComponentUI::DrawFloatControl("Rest. Threshold", component.RestitutionThreshold, .01f, false, .0f);
			});
		
		ImGui::PopID();
	}
	
	void SceneHierarchyPanel::AddComponentsModal(Entity entity)
	{
		Entity selected = Editor::GetSelectedEntity();
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
				clicked |= ComponentUI::AddComponentMenuItem<SpriteComponent>("Sprite", selected);
				clicked |= ComponentUI::AddComponentMenuItem<CircleComponent>("Circle", selected);
				clicked |= ComponentUI::AddComponentMenuItem<ScriptComponent>("Script", selected);
				clicked |= ComponentUI::AddComponentMenuItem<CameraComponent>("Camera", selected);
				clicked |= ComponentUI::AddComponentMenuItem<RigidBody2DComponent>("2D Rigid Body", selected, Editor::GetSceneState() == SceneState::Edit);
				clicked |= ComponentUI::AddComponentMenuItem<RectColliderComponent>("2D Rectangular Collider", selected, Editor::GetSceneState() == SceneState::Edit);
				clicked |= ComponentUI::AddComponentMenuItem<CircleColliderComponent>("2D Circular Collider", selected, Editor::GetSceneState() == SceneState::Edit);

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

