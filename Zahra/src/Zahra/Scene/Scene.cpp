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
		SceneCamera* activeCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;

		auto cameraEntities = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : cameraEntities)
		{
			auto& [transform, camera] = cameraEntities.get<TransformComponent, CameraComponent>(entity);
			if (camera.active)
			{
				activeCamera = &camera.Camera;
				cameraTransform = &transform.Transform;
				break;
			}
			
		}

		if (activeCamera)
		{
			auto spriteEntities = m_Registry.view<TransformComponent, SpriteComponent>();

			Renderer2D::BeginScene(activeCamera->GetProjection(), *cameraTransform);

			for (auto entity : spriteEntities)
			{
				auto& [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

				Renderer2D::DrawQuad(transform, sprite.Colour);
			}

			Renderer2D::EndScene();			
		}
	}

	void Scene::OnViewportResize(float width, float height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto cameraEntities = m_Registry.view<CameraComponent>();
		for (auto entity : cameraEntities)
		{
			auto& cameraComponent = cameraEntities.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

}

