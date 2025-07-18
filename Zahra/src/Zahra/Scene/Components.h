#pragma once

#include "Zahra/Core/UUID.h"
#include "Zahra/Renderer/Cameras/SceneCamera.h"
#include "Zahra/Renderer/Texture.h"
#include "Zahra/Renderer/Text/Font.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
	#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/quaternion.hpp>

namespace Zahra
{
	// For every component struct added, make sure to do the following:
	// 1) give it a default constructor and copy constructor
	// 2) add to registry (bottom of this header)
	// 3) include ComponentUI::DrawComponent and AddComponentsModal UI code (SceneHierarchyPanel.cpp)
	// 4) add serialisation code (SceneSerialiser.cpp)
	// 5) write a corresponding class in Djinn (Components.cs)
	// 6) connect script get/set functions (ScriptGlue.cpp & InternalCalls.cs)

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CORE COMPONENTS

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const UUID& uuid)
			: ID(uuid) {}

	};

	struct HierarchyComponent
	{
		UUID Parent;
		std::vector<UUID> Children;

		HierarchyComponent() = default;
		HierarchyComponent(const HierarchyComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { .0f, .0f, .0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), Translation)
				 * glm::toMat4(Quaternion)
				 * glm::scale(glm::mat4(1.0f), Scale);
		}

		glm::mat4 GetRotation() const
		{
			return glm::toMat4(Quaternion);
		}

		glm::vec3 GetEulers() const
		{
			return EulerAngles;
		}

		glm::quat GetQuaternion() const
		{
			return Quaternion;
		}

		void SetRotation(const glm::vec3& eulerAngles)
		{
			EulerAngles = eulerAngles;
			Quaternion = glm::quat(eulerAngles);
		}

		// TODO: decompose quaternion into euler angle representation, attempting to maintain
		// continuity with current euler angle values i.e. no sudden "flips"
		/*void SetRotation(const glm::quat& quaternion)
		{
			Quaternion = quaternion;
			
			???
		}*/

	private:
		glm::vec3 EulerAngles = { 0.f, 0.f, 0.f };
		glm::quat Quaternion = { 1.f, 0.f, 0.f, 0.f };

	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// RENDERING COMPONENTS

	struct SpriteComponent
	{
		glm::vec4 Tint{ 1.0f, 1.0f, 1.0f, 1.0f };
		AssetHandle TextureHandle = 0;
		float TextureTiling = 1.0f;
		bool Animated = false;

		// TODO: animation!!

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(const glm::vec4& tint)
			: Tint(tint) {}

	};

	struct CircleComponent
	{
		glm::vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = .005f;

		CircleComponent() = default;
		CircleComponent(const CircleComponent&) = default;
		CircleComponent(const glm::vec4& colour)
			: Colour(colour) {}

	};

	struct TextComponent
	{
		std::string String;
		Ref<Font> Font; // TODO: replace with asset handle
		glm::vec4 FillColour;
		glm::vec4 BackgroundColour;

		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
		// TODO: add constructor from string + font asset handle
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(bool fixedRatio)
			: FixedAspectRatio(fixedRatio) {}

	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SCRIPTING COMPONENTS

	struct ScriptComponent
	{
		std::string ScriptName = "None";

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
		ScriptComponent(const std::string& scriptName)
			: ScriptName(scriptName) {}
	};

	class NativeScriptableEntity;

	struct NativeScriptComponent
	{
		// TODO: this should be private ultimately
		NativeScriptableEntity* Instance = nullptr;

		NativeScriptableEntity* (*InstantiateScript)();
		void(*DestroyScript)(NativeScriptComponent*);

		NativeScriptComponent() = default;
		NativeScriptComponent(const NativeScriptComponent&) = default;

		template <typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<NativeScriptableEntity*>(new T()); }; // Note this requires a default constructor
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}

	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PHYSICS COMPONENTS

	struct RigidBody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		
		// TODO: add other body attributes from b2Body e.g. b2ContactListener for callbacks on collision
		bool FixedRotation = false;

		// TODO: remove this in favour of being able to retrieve this from the
		// physics engine, using the entity uuid
		void* RuntimeBody = nullptr;

		RigidBody2DComponent() = default;
		RigidBody2DComponent(const RigidBody2DComponent&) = default;

	};

	struct RectColliderComponent
	{
		glm::vec2 Offset = { .0f, .0f };
		glm::vec2 HalfExtent = { .5f, .5f };

		// TODO: investigate physically reasonable default values
		float Density = 1.0f;
		float Friction = .5f;
		float Restitution = .5f;
		float RestitutionThreshold = .5f;

		RectColliderComponent() = default;
		RectColliderComponent(const RectColliderComponent&) = default;

	};

	struct CircleColliderComponent
	{
		glm::vec2 Offset = { .0f, .0f };
		float Radius = .5f;

		// TODO: investigate physically reasonable default values
		float Density = 1.0f;
		float Friction = .5f;
		float Restitution = .5f;
		float RestitutionThreshold = .5f;

		CircleColliderComponent() = default;
		CircleColliderComponent(const CircleColliderComponent&) = default;

	};

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// COMPONENT REGISTRY

	template<typename... Component>
	struct ComponentGroup
	{

	};

	using MostComponents = ComponentGroup<
		TransformComponent,
		SpriteComponent, CircleComponent, TextComponent,
		CameraComponent,
		ScriptComponent,
		RigidBody2DComponent, RectColliderComponent, CircleColliderComponent
	>;

}

