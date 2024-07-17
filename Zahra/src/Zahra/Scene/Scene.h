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
		void DestroyEntity(Entity entity);

		void OnUpdate(float dt);

		void OnViewportResize(float width, float height);

		const std::string& GetName() { return m_SceneName; }
		void SetName(const std::string& name) { m_SceneName = name; }


	private:
		std::string m_SceneName;

		entt::basic_registry<entt::entity> m_Registry; // TODO: custom UUIDs (change the template param)
		float m_ViewportWidth = 1.0f, m_ViewportHeight = 1.0f;

		// TODO: find a better way of doing this!
		template<typename T>
		void OnComponentAdded(Entity entity, T& component); // must specialise this for each component type in use.

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class PropertiesPanel;
		friend class SceneSerialiser;
	};

}


