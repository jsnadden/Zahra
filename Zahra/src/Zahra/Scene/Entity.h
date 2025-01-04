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
			if (m_EntityHandle == entt::null)
				return false;

			if (strict)
				return m_Scene->m_Registry.all_of<Types...>(m_EntityHandle);
			else
				return m_Scene->m_Registry.any_of<Types...>(m_EntityHandle);
		}

		template<typename T, typename ...Args>
		T& AddComponent(Args&& ...args)
		{
			Z_CORE_ASSERT(!HasComponents<T>(), "Entity already has a component of this type.");
			
			T& component =  m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			
			return component;
		}

		template<typename T, typename ...Args>
		T& AddOrReplaceComponent(Args&& ...args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);

			return component;
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

		ZGUID GetGUID()
		{
			Z_CORE_ASSERT(HasComponents<IDComponent>(), "Entity does not have an IDComponent.");

			return GetComponents<IDComponent>().ID;
		}

		const std::string& GetName()
		{
			Z_CORE_ASSERT(HasComponents<TagComponent>(), "Entity does not have a TagComponent.");

			return GetComponents<TagComponent>().Tag;
		}

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator==(const Entity& other) const
		{
			return (m_EntityHandle == other.m_EntityHandle) && (m_Scene == other.m_Scene);
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr; // TODO: Should be a WeakRef
	};

}


