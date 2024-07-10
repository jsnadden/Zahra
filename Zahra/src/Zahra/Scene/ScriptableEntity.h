#pragma once

#include "Entity.h"

namespace Zahra
{

	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() {}

		template<typename ...Types>
		auto& GetComponents()
		{
			return m_Entity.GetComponents<Types...>();
		}

	protected:
		virtual void OnCreate() {};
		virtual void OnDestroy() {};
		virtual void OnUpdate(float dt) {};

	private:
		Entity m_Entity;

		friend class Scene;
	};


}