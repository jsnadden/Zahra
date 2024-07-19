#include "zpch.h"
#include "Scene.h"

#include "Zahra/Renderer/Renderer.h"
#include "Entity.h"
#include "ScriptableEntity.h"

namespace Zahra
{
	
	Scene::Scene()
	{
		m_SceneName = "Untitled";
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

	



	void Scene::OnUpdateEditor(float dt, EditorCamera& camera)
	{
		auto spriteEntities = m_Registry.view<TransformComponent, SpriteComponent>();

		Renderer::BeginScene(camera);

		for (auto entity : spriteEntities)
		{
			auto [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

			Renderer::DrawQuad(transform.GetTransform(), sprite.Colour);
		}

		Renderer::EndScene();
	}

	void Scene::OnUpdateRuntime(float dt)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RUN SCRIPTS
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nativeScript)
				{
					// TODO: move instantiation to Scene::OnScenePlay(), when that exists
					if (!nativeScript.Instance)
					{
						nativeScript.Instance = nativeScript.InstantiateScript();
						nativeScript.Instance->m_Entity = Entity{ entity, this };
						nativeScript.Instance->OnCreate();
					}

					nativeScript.Instance->OnUpdate(dt);
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

				Renderer::BeginScene(activeCamera->GetProjection(), cameraTransform);

				for (auto entity : spriteEntities)
				{
					auto [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

					Renderer::DrawQuad(transform.GetTransform(), sprite.Colour);
				}

				Renderer::EndScene();
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

	Entity Scene::GetActiveCamera()
	{
		// TODO: we really need a better way of doing this :(
		auto view = m_Registry.view<CameraComponent>();
		
		for (auto entity : view)
		{
			auto camera = view.get<CameraComponent>(entity);
			if (camera.Active) return Entity{ entity, this };
		}

		return {};
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

