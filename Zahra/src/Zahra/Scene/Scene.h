#pragma once

#include "Components.h"

#include <entt.hpp>

namespace Zahra
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		// TODO: need to make our own wrapper Entity class, so that client apps aren't interfacing with entt directly
		// (also makes it easier to swap out the specific ECS implementation for one of our own)
		entt::entity CreateEntity();

		// TODO: remove this asap
		entt::registry& REG() { return m_Registry; }

		void OnUpdate(float dt);

	private:
		entt::registry m_Registry;

	};

}


