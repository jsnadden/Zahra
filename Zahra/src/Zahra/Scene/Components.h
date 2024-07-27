#pragma once

#include "SceneCamera.h"
#include "Zahra/Renderer/Texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Zahra
{
	// Every component struct must have
	// 1) default constructor/copy constructors
	// 2) static const bool Essential = true/false (essential components can never be removed from an entity)

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag )
			: Tag(tag) {}

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }

		static const bool Essential = true;
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

		static const bool Essential = true;
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

		
		// TODO: don't forget to add stuff to the sceneserialiser methods!!

		static const bool Essential = false;
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


		static const bool Essential = false;
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

		static const bool Essential = false;
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

