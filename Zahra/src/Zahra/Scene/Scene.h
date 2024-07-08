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

	private:
		entt::basic_registry<entt::entity> m_Registry; // TODO: custom UUIDs (change the template param)



		friend class Entity;
	};

}


