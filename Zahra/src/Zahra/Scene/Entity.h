#pragma once

#include "Scene.h"

#include <entt.hpp>

namespace Zahra
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& entity) = default;
		~Entity() = default;

		template<typename ...Types>
		bool HasComponents(bool strict = true)
		{
			if (strict)
				return m_Scene->m_Registry.all_of<Types...>(m_EntityHandle);
			else
				return m_Scene->m_Registry.any_of<Types...>(m_EntityHandle);
		}

		template<typename T, typename ...Args>
		T& AddComponent(Args&& ...args)
		{
			Z_CORE_ASSERT(!HasComponents<T>(), "Entity already has a component of this type.");
			
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		void RemoveComponent()
		{
			Z_CORE_ASSERT(HasComponents<T>(), "Entity does not have component of requested type.");

			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		template<typename ...Types>
		auto& GetComponents()
		{
			Z_CORE_ASSERT(HasComponents<Types...>(), "Entity does not have components of the requested types.");

			return m_Scene->m_Registry.get<Types...>(m_EntityHandle);
		}

	private:
		entt::entity m_EntityHandle{ 0 };
		Scene* m_Scene = nullptr; // TODO: Should be a weak (aka non-owning) reference. Once again, need our own reference counting!
	};

}


