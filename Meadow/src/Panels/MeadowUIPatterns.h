#pragma once

#include "Zahra/Scene/Entity.h"

#include <ImGui/imgui_internal.h>
#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Zahra
{

	namespace MeadowUIPatterns
	{
		template <typename T, typename UIFunction>
		void DrawComponent(const std::string& name, Entity entity, UIFunction ComponentInterface)
		{
			if (entity.HasComponents<T>())
			{
				const ImGuiTreeNodeFlags nodeFlags =
					ImGuiTreeNodeFlags_DefaultOpen |
					ImGuiTreeNodeFlags_AllowItemOverlap |
					ImGuiTreeNodeFlags_SpanAvailWidth |
					ImGuiTreeNodeFlags_Framed;

				auto& component = entity.GetComponents<T>();

				bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), nodeFlags, name.c_str());

				bool removedComponent = false;
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Remove component", 0, false, !std::is_same<T, TransformComponent>::value)) removedComponent = true;

					ImGui::EndPopup();
				}

				if (open)
				{
					if (ImGui::BeginTable(typeid(T).name(), 2))
					{
						ImGui::TableSetupColumn("labels", ImGuiTableColumnFlags_WidthFixed, 150.f); // TODO: figure out a nice column width policy
						ImGui::TableSetupColumn("controls");
						
						ComponentInterface(component);

						ImGui::EndTable();
					}
					
					ImGui::TreePop();
				}

				if (removedComponent) entity.RemoveComponent<T>();
			}
		}

		template <typename T>
		void AddComponentMenuItem(const char* name, Entity entity)
		{
			if (ImGui::MenuItem(name, 0, false, !entity.HasComponents<T>()))
			{
				entity.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}

		void DrawFloatControl(const std::string& label, float& value, float speed = .05f,
			bool logarithmic = false, float min = -FLT_MAX / INT_MAX, float max = FLT_MAX / INT_MAX)
		{
			ImGuiSliderFlags flags = 0;

			if (logarithmic)
			{
				flags = 32 & 64;
				min = speed;
			}

			ImGui::PushID(label.c_str());

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::TableNextColumn();
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
			}

			ImGui::TableNextColumn();
			{
				ImGui::PushItemWidth(ImGui::CalcItemWidth());
				ImGui::DragFloat("##X", &value, speed, min, max, "%.2f", flags);
				ImGui::PopItemWidth();
			}

			ImGui::PopID();

		}

		void DrawFloat2Controls(const std::string& label, glm::vec2& values, float resetValue = .0f, float speed = .05f,
			bool logarithmic = false, float min = -FLT_MAX / INT_MAX, float max = FLT_MAX / INT_MAX)
		{
			ImGuiSliderFlags flags = 0;

			if (logarithmic)
			{
				flags = 32 & 64;
				min = speed;
			}

			ImGuiIO& io = ImGui::GetIO();
			auto regularFont = io.Fonts->Fonts[0];
			auto boldFont = io.Fonts->Fonts[1];

			ImGui::PushID(label.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::TableNextColumn();

			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
			}

			ImGui::TableNextColumn();

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2);
			ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());

			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.8f, .1f, .15f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.9f, .2f, .20f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.8f, .1f, .15f, 1.0f));
				ImGui::PushFont(boldFont);
				if (ImGui::Button("X", buttonSize)) values.x = resetValue;
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

				ImGui::SameLine();

				ImGui::DragFloat("##X", &values.x, speed, min, max, "%.2f", flags);

				ImGui::PopItemWidth();
			}
			ImGui::SameLine();
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.2f, .7f, .2f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.3f, .8f, .3f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.2f, .7f, .2f, 1.0f));
				ImGui::PushFont(boldFont);
				if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

				ImGui::SameLine();

				ImGui::DragFloat("##Y", &values.y, speed, min, max, "%.2f", flags);

				ImGui::PopItemWidth();
			}

			ImGui::PopStyleVar(2);
			ImGui::PopID();

		}

		void DrawFloat3Controls(const std::string& label, glm::vec3& values, float resetValue = .0f, float speed = .05f,
			bool logarithmic = false, float min = -FLT_MAX / INT_MAX, float max = FLT_MAX / INT_MAX)
		{
			ImGuiSliderFlags flags = 0;

			if (logarithmic)
			{
				flags = 32 & 64;
				min = speed;
			}

			ImGuiIO& io = ImGui::GetIO();
			auto regularFont = io.Fonts->Fonts[0];
			auto boldFont = io.Fonts->Fonts[1];

			ImGui::PushID(label.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::TableNextColumn();

			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
			}

			ImGui::TableNextColumn();

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2);
			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.8f, .1f, .15f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.9f, .2f, .20f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.8f, .1f, .15f, 1.0f));
				ImGui::PushFont(boldFont);
				if (ImGui::Button("X", buttonSize)) values.x = resetValue;
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

				ImGui::SameLine();

				ImGui::DragFloat("##X", &values.x, speed, min, max, "%.2f", flags);

				ImGui::PopItemWidth();
			}
			ImGui::SameLine();
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.2f, .7f, .2f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.3f, .8f, .3f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.2f, .7f, .2f, 1.0f));
				ImGui::PushFont(boldFont);
				if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

				ImGui::SameLine();

				ImGui::DragFloat("##Y", &values.y, speed, min, max, "%.2f", flags);

				ImGui::PopItemWidth();
			}
			ImGui::SameLine();
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.1f, .25f, .8f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.2f, .35f, .9f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.1f, .25f, .8f, 1.0f));
				ImGui::PushFont(boldFont);
				if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

				ImGui::SameLine();

				ImGui::DragFloat("##Z", &values.z, speed, min, max, "%.2f", flags);

				ImGui::PopItemWidth();
			}

			ImGui::PopStyleVar(2);
			ImGui::PopID();

		}

		void DrawRGBAControl(const std::string& label, glm::vec4& values)
		{
			ImGui::PushID(label.c_str());

			ImGui::TableNextColumn();

			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
			}

			ImGui::TableNextColumn();
						
			{
				ImGui::ColorEdit4("##RGBA", glm::value_ptr(values));
			}

			ImGui::PopID();

		}

		void DrawBoolControl(const std::string& label, bool& value)
		{
			ImGui::PushID(label.c_str());

			ImGui::TableNextColumn();

			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
			}

			ImGui::TableNextColumn();

			{
				ImGui::Checkbox("##bool", &value);
			}

			ImGui::PopID();

		}

		int DrawComboControl(const std::string& label, const char** options, int count, int currentValue)
		{
			int newValue = currentValue;

			ImGui::PushID(label.c_str());

			ImGui::TableNextColumn();

			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
			}

			ImGui::TableNextColumn();

			{
				if (ImGui::BeginCombo("##options", options[currentValue]))
				{
					for (int i = 0; i < count; i++)
					{
						if (ImGui::Selectable(options[i], currentValue == i))
						{
							newValue = i;
						}

						if (currentValue == i) ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}
			}

			ImGui::PopID();

			return newValue;
		}

		void DrawTextureDrop(const std::string& label, Ref<Texture2D>& texture)
		{
			ImGui::PushID(label.c_str());

			ImGui::TableNextColumn();

			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
			}

			ImGui::TableNextColumn();

			{
				ImGui::Button("drop texture here"); // TODO: redesign this once we have a texture manager. Ideally would display texture name!

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BROWSER_FILE_IMAGE"))
					{
						char filepath[256];
						strcpy_s(filepath, (const char*)payload->Data);

						texture = Texture2D::Create(filepath);
					}

					ImGui::EndDragDropTarget();
				}
			}

			ImGui::PopID();
		}
	};

}
