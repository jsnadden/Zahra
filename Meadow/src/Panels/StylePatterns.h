#pragma once

#include "Zahra/Scene/Entity.h"

#include <ImGui/imgui_internal.h>
#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Zahra
{

	class StylePatterns
	{
	public:

		static void DrawVec3Controls(const std::string& label, glm::vec3& values, float resetValue = .0f, float speed = .05f,
			bool logarithmic = false, float min = -FLT_MAX / INT_MAX, float max = FLT_MAX / INT_MAX, float columnWidth = 90.f)
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

			if (ImGui::BeginTable(label.c_str(), 2))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
				ImGui::TableSetupColumn("sliders");

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
				ImGui::EndTable();
				ImGui::PopID();
			}

		}

		template <typename T, typename UIFunction>
		static void DrawComponent(const std::string& name, Entity entity, UIFunction ComponentInterface)
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
					if (ImGui::MenuItem("Remove component", 0, false, !T::Essential)) removedComponent = true;

					ImGui::EndPopup();
				}

				if (open)
				{
					ComponentInterface(component);
					ImGui::TreePop();
				}

				if (removedComponent) entity.RemoveComponent<T>();
			}
		}

	};

}
