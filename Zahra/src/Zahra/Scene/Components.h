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
	// 1) the component MUST declare a default constructor/copy constructor
	// 2) add component properties UI code, and add/remove context menus, in SceneHierarchyPanel.cpp
	// 3) add component serialisation code in SceneSerialiser.cpp
	//
	// [TODO: reflection should automate 2) and 3)]

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

	struct CameraComponent
	{
		SceneCamera Camera;
		bool FixedAspectRatio = false;

		// TODO: this status should be held by a scene, not an entity! (maybe the scene can store the active camera's UUID?)
		bool Active = true;

		void Test()
		{
			Z_CORE_INFO("created camera");
		}

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(bool fixedRatio)
			: FixedAspectRatio(fixedRatio) {}

	};

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

	class ScriptableEntity;

	struct NativeScriptComponent
	{
		// TODO: this should be private ultimately
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity*(*InstantiateScript)();
		void(*DestroyScript)(NativeScriptComponent*);

		NativeScriptComponent() = default;
		NativeScriptComponent(const NativeScriptComponent&) = default;

		template <typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); }; // Note this requires a default constructor (no params)
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}

	};

	//class CameraController : public ScriptableEntity
		//{
		//public:
		//	void OnUpdate(float dt)
		//	{
		//		auto& position = GetComponents<TransformComponent>().Translation;
		//		if (HasComponents<CameraComponent>())
		//		{
		//			auto& camera = GetComponents<CameraComponent>().Camera;
		//			float speed = camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic
		//				? .5f * camera.GetOrthographicSize() : 2.0f;
	    //
		//			if (Input::IsKeyPressed(KeyCode::A))
		//				position.x -= speed * dt;
		//			if (Input::IsKeyPressed(KeyCode::D))
		//				position.x += speed * dt;
		//			if (Input::IsKeyPressed(KeyCode::W))
		//				position.y += speed * dt;
		//			if (Input::IsKeyPressed(KeyCode::S))
		//				position.y -= speed * dt;
		//		}
		//	}
		//};

}

