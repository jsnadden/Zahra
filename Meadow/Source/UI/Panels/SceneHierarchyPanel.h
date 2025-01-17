#pragma once

#include "Editor/TypeDefs.h"

#include <Zahra.h>

namespace Zahra
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();

		void OnImGuiRender();

		void CacheScriptClassNames();

	private:
		bool m_ShowAddComponentsModal = false;

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		void AddComponentsModal(Entity entity);

		std::vector<const char*> m_ScriptClassNames;
		uint32_t m_ScriptClassCount = 0;
	};

}


