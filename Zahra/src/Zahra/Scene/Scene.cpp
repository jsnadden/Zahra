#include "zpch.h"
#include "Scene.h"

#include "Zahra/Renderer/Renderer.h"
#include "Entity.h"
#include "ScriptableEntity.h"
#include "Components.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>

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
		// TODO: is there more to this cleanup?
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		// TODO: implement this, using:
		// https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#meet-the-runtime
		// https://github.com/skypjack/entt/issues/694
		// add "(copy)" to the tag?

		return Entity();
	}

	void Scene::OnRuntimePlay()
	{
		InitPhysicsWorld();

		// Instantiate scripts
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nativeScript)
			{
				if (!nativeScript.Instance)
				{
					nativeScript.Instance = nativeScript.InstantiateScript();
					nativeScript.Instance->m_Entity = Entity{ entity, this };
					nativeScript.Instance->OnCreate();
				}
			});
	}

	void Scene::OnRuntimeStop()
	{
		m_PhysicsWorld = nullptr;
		// TODO: currently not clearing m_PhysicsBodies, because this would
		// call their destructors, which for some reason are private in box2D.
		// Hopefully this isn't a memory issue :S
	}

	void Scene::OnUpdateEditor(float dt, EditorCamera& camera)
	{
		auto spriteEntities = m_Registry.view<TransformComponent, SpriteComponent>();

		Renderer::BeginScene(camera);

		for (auto entity : spriteEntities)
		{
			auto [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

			Renderer::DrawSprite(transform.GetTransform(), sprite, (int)entity);
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
					nativeScript.Instance->OnUpdate(dt);
				});
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RUN PHYSICS SIMULATION
		{
			// TODO: fine-tune these solver parameters, maybe expose to editor?
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;

			m_PhysicsWorld->Step(dt, velocityIterations, positionIterations);

			// Retrieve resultant transforms
			auto view = m_Registry.view<RigidBody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& transformComp = entity.GetComponents<TransformComponent>();
				auto& bodyComp = entity.GetComponents<RigidBody2DComponent>();

				const auto& position = m_PhysicsBodies[e]->GetPosition();
				const auto& rotation = m_PhysicsBodies[e]->GetAngle();

				transformComp.Translation.x = position.x;
				transformComp.Translation.y = position.y;
				transformComp.EulerAngles.z = rotation;
			}
		}
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RENDER SCENE
		{
			SceneCamera* activeCamera = nullptr;
			glm::mat4 cameraTransform;

			// TODO: find a better way of getting a camera. this just picks the first one it finds
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

					Renderer::DrawSprite(transform.GetTransform(), sprite);
				}

				Renderer::EndScene();
			}
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	static b2BodyType ZRigidBodyTypeToBox2D(RigidBody2DComponent::BodyType type)
	{
		switch (type)
		{
			case RigidBody2DComponent::BodyType::Static:
				return b2BodyType::b2_staticBody;
			
			case RigidBody2DComponent::BodyType::Dynamic:
				return b2BodyType::b2_dynamicBody;

			case RigidBody2DComponent::BodyType::Kinematic:
				return b2BodyType::b2_kinematicBody;
		}

		Z_CORE_ASSERT(false, "Invalid BodyType value");
		return b2BodyType::b2_staticBody; // just to prevent warnings *eye roll*
	}

	void Scene::InitPhysicsWorld()
	{
		b2Vec2 gravity = { .0f, -9.8f }; // TODO: make this setable?
		m_PhysicsWorld = std::make_unique<b2World>(gravity);

		auto view = m_Registry.view<RigidBody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transformComp = entity.GetComponents<TransformComponent>();
			auto& bodyComp = entity.GetComponents<RigidBody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = ZRigidBodyTypeToBox2D(bodyComp.Type);
			bodyDef.position.Set(transformComp.Translation.x, transformComp.Translation.y);
			bodyDef.angle = transformComp.EulerAngles.z;

			m_PhysicsBodies[e] = m_PhysicsWorld->CreateBody(&bodyDef);
			m_PhysicsBodies[e]->SetFixedRotation(bodyComp.FixedRotation);

			// it makes sense to have bodies with no colliders, but not the other way
			// around. Also the collider could be of different types. Therefore it
			// makes sense to ask for these components optionally, and secondarily.
			if (entity.HasComponents<RectColliderComponent>())
			{
				auto& collider = entity.GetComponents<RectColliderComponent>();

				b2PolygonShape shape;
				shape.SetAsBox(transformComp.Scale.x * collider.HalfExtent.x, transformComp.Scale.y * collider.HalfExtent.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = collider.Density;
				fixtureDef.friction = collider.Friction;
				fixtureDef.restitution = collider.Restitution;
				fixtureDef.restitutionThreshold = collider.RestitutionThreshold;

				m_PhysicsBodies[e]->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::UpdatePhysicsWorld()
	{

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
	
	// TODO: scrap the following nonsense. Currently only the cameracomponent utilises
	// it, and otherwise it just leads to opaque compiler errors when forgotton

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
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template <>
	void Scene::OnComponentAdded<SpriteComponent>(Entity entity, SpriteComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<RectColliderComponent>(Entity entity, RectColliderComponent& component)
	{}

	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{}

}

