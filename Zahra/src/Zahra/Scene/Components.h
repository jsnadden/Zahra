#pragma once

#include "SceneCamera.h"
#include "Zahra/Renderer/Texture.h"
#include "Zahra/Core/GUID.h"

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
	// 3) include MeadowUIPatterns::DrawComponent and AddComponentsModal UI code (SceneHierarchyPanel.cpp)
	// 4) add serialisation code (SceneSerialiser.cpp)
	// 5) write a corresponding class in Djinn (Components.cs)
	// 6) connect script get/set functions (ScriptGlue.cpp & InternalCalls.cs)

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CORE COMPONENTS

	struct IDComponent
	{
		ZGUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const ZGUID& guid)
			: ID(guid) {}

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
		glm::vec3 EulerAngles = { .0f, .0f, .0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), Translation)
				 * glm::toMat4(glm::quat(EulerAngles))
				 * glm::scale(glm::mat4(1.0f), Scale);
		}

	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// RENDERING COMPONENTS

	struct SpriteComponent
	{
		glm::vec4 Tint{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr; // TODO: material system?
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

	struct CameraComponent
	{
		SceneCamera Camera;
		bool FixedAspectRatio = false;

		// TODO: this status should be held by a scene, not an entity! (maybe the scene can store the active camera's UUID?)
		bool Active = true;

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
			InstantiateScript = []() { return static_cast<NativeScriptableEntity*>(new T()); }; // Note this requires a default constructor (no params)
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

		RigidBody2DComponent() = default;
		RigidBody2DComponent(const RigidBody2DComponent&) = default;

	};

	struct RectColliderComponent // TODO: triangles and circles?
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

	using AllComponents = ComponentGroup<
		TransformComponent,
		SpriteComponent, CircleComponent,
		CameraComponent,
		ScriptComponent,
		RigidBody2DComponent, RectColliderComponent, CircleColliderComponent
	>;

}

