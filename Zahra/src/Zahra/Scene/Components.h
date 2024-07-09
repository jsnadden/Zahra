#pragma once

#include "SceneCamera.h"

#include <glm/glm.hpp>

namespace Zahra
{

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag )
			: Tag(tag) {}

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }

	};

	struct TransformComponent
	{
		glm::mat4 Transform{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& transform)
			: Transform(transform) {}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }

	};

	struct SpriteComponent
	{
		glm::vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(const glm::vec4& colour)
			: Colour(colour) {}

		// TODO: add textures, a "sprite type" enum etc.
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool FixedAspectRatio = false;

		// TODO: this status should be held by a scene, not an entity!
		bool active = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(bool fixedRatio)
			: FixedAspectRatio(fixedRatio) {}

	};

}

