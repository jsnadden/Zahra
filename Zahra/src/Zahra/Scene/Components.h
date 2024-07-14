#pragma once

#include "SceneCamera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), EulerAngles.x, glm::vec3(1.0f, 0.0f, 0.0f))
							   * glm::rotate(glm::mat4(1.0f), EulerAngles.y, glm::vec3(0.0f, 1.0f, 0.0f))
							   * glm::rotate(glm::mat4(1.0f), EulerAngles.z, glm::vec3(0.0f, 0.0f, 1.0f));

			return glm::translate(glm::mat4(1.0f), Translation)
				 * rotation
				 * glm::scale(glm::mat4(1.0f), Scale);
		}

		static const bool Essential = true;
	};

	struct SpriteComponent
	{
		glm::vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(const glm::vec4& colour)
			: Colour(colour) {}

		// TODO: add textures, a "sprite type" enum etc.

		static const bool Essential = false;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool FixedAspectRatio = false;

		// TODO: this status should be held by a scene, not an entity!
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

}

