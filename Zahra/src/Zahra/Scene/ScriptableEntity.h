#pragma once

#include "Entity.h"

namespace Zahra
{

	class ScriptableEntity
	{
	public:
		template<typename ...Types>
		auto& GetComponents()
		{
			return m_Entity.GetComponents<Types...>();
		}

	private:
		Entity m_Entity;

		friend class Scene;
	};


}