#include "zpch.h"
#include "Scene.h"

#include "Zahra/Renderer/Renderer2D.h"

namespace Zahra
{
	
	Scene::Scene()
	{
		entt::entity entity = m_Registry.create();
	}

	Scene::~Scene()
	{

	}

	entt::entity Scene::CreateEntity()
	{
		// TODO: need to make our own wrapper Entity class, so that client apps aren't interfacing with entt directly
		// (also makes it easier to swap out the specific ECS implementation for one of our own)
		return m_Registry.create();
	}

	void Scene::OnUpdate(float dt)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);

		for (auto entity : group)
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);

			Renderer2D::DrawQuad(transform, sprite.Colour);
		}
	}

}

