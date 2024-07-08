#include "zpch.h"
#include "Scene.h"

#include "Zahra/Renderer/Renderer2D.h"
#include "Entity.h"

namespace Zahra
{
	
	Scene::Scene()
	{
		
	}

	Scene::~Scene()
	{
		m_Registry.clear();
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>(name.empty() ? "anonymous_entity" : name);
		return entity;
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

