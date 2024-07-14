#include "zpch.h"
#include "Scene.h"

#include "Zahra/Renderer/Renderer2D.h"
#include "Entity.h"
#include "ScriptableEntity.h"

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

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	

	void Scene::OnUpdate(float dt)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RUN SCRIPTS
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
				{
					// TODO: move instantiation to Scene::OnScenePlay(), when that exists
					if (!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = Entity{ entity, this };
						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(dt);
				});
		}
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RENDER SCENE
		{
			SceneCamera* activeCamera = nullptr;
			glm::mat4 cameraTransform;

			// TODO: destroy this, it's truly disgusting
			auto cameraEntities = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : cameraEntities)
			{
				auto [transform, camera] = cameraEntities.get<TransformComponent, CameraComponent>(entity);
				if (camera.Active)
				{
					activeCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}

			}

			if (activeCamera)
			{
				auto spriteEntities = m_Registry.view<TransformComponent, SpriteComponent>();

				Renderer2D::BeginScene(activeCamera->GetProjection(), cameraTransform);

				for (auto entity : spriteEntities)
				{
					auto [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

					Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour);
				}

				Renderer2D::EndScene();
			}
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false); // must specialise this for each component type in use
	}

	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template <>
	void Scene::OnComponentAdded<SpriteComponent>(Entity entity, SpriteComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{}

}

