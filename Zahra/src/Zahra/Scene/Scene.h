#pragma once

#include "Components.h"

#include <entt.hpp>

namespace Zahra
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "anonymous_entity");

		void OnUpdate(float dt);

		void OnViewportResize(float width, float height);

	private:
		entt::basic_registry<entt::entity> m_Registry; // TODO: custom UUIDs (change the template param)
		float m_ViewportWidth = 1.0f, m_ViewportHeight = 1.0f;

		friend class Entity;
		friend class SceneHierarchyPanel;
	};

}


