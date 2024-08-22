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
#include <box2d/b2_circle_shape.h>

namespace Zahra
{

	Scene::Scene()
	{
		m_SceneName = "Untitled";

		m_Registry.on_construct<entt::entity>().connect<&entt::registry::emplace_or_replace<IDComponent>>();
		m_Registry.on_construct<entt::entity>().connect<&entt::registry::emplace_or_replace<TagComponent>>();
		m_Registry.on_construct<entt::entity>().connect<&entt::registry::emplace_or_replace<TransformComponent>>();
		m_Registry.on_construct<CameraComponent>().connect<&Scene::InitCameraComponent>(this);
	}

	Scene::~Scene()
	{
		m_Registry.clear();
	}

	template<typename ComponentType>
	static void CopyComponent(entt::registry& srcReg, entt::registry& destReg, std::unordered_map<ZGUID, entt::entity> guidToNewHandle)
	{

		auto componentView = destReg.view<ComponentType>();
		for (auto oldHandle : componentView)
		{
			ZGUID guid = destReg.get<IDComponent>(oldHandle).ID;
			entt::entity newHandle = guidToNewHandle[guid];

			auto& oldComponent = destReg.get<ComponentType>(oldHandle);
			srcReg.emplace_or_replace<ComponentType>(newHandle, oldComponent);
		}

	}

	template <typename ComponentType>
	static void CopyComponentIfExists(Entity srcEnt, Entity destEnt)
	{
		if (!srcEnt.HasComponents<ComponentType>()) return;
		
		destEnt.AddOrReplaceComponent<ComponentType>(srcEnt.GetComponents<ComponentType>());
	}

	Ref<Scene> Scene::CopyScene(Ref<Scene> oldScene)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = oldScene->m_ViewportWidth;
		newScene->m_ViewportHeight = oldScene->m_ViewportHeight;

		auto& oldRegistry = oldScene->m_Registry;
		auto& newRegistry = newScene->m_Registry;

		std::unordered_map<ZGUID, entt::entity> guidToNewHandle;

		// TODO: figure out how to simplify this using entt (meta or snapshot)

		// copy entities
		oldRegistry.view<entt::entity>().each([&](auto entityHandle)
			{
				Entity oldEntity = { entityHandle, oldScene.get() };
				ZGUID guid = oldEntity.GetGUID();
				guidToNewHandle[guid] = (entt::entity)newScene->CreateEntity(guid, oldEntity.GetComponents<TagComponent>().Tag);
			});

		// copy components
		CopyComponent<TransformComponent>		(newRegistry, oldRegistry, guidToNewHandle);
		CopyComponent<SpriteComponent>			(newRegistry, oldRegistry, guidToNewHandle);
		CopyComponent<CircleComponent>			(newRegistry, oldRegistry, guidToNewHandle);
		CopyComponent<CameraComponent>			(newRegistry, oldRegistry, guidToNewHandle);
		CopyComponent<RigidBody2DComponent>		(newRegistry, oldRegistry, guidToNewHandle);
		CopyComponent<RectColliderComponent>	(newRegistry, oldRegistry, guidToNewHandle);
		CopyComponent<CircleColliderComponent>	(newRegistry, oldRegistry, guidToNewHandle);
		CopyComponent<NativeScriptComponent>	(newRegistry, oldRegistry, guidToNewHandle);

		return newScene;
	}

	// All entities will be created with an IDComponent, TagComponent and TransformComponent
	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.GetComponents<TagComponent>().Tag = name.empty() ? "unnamed_entity" : name;
		return entity;
	}

	Entity Scene::CreateEntity(uint64_t guid, const std::string& name)
	{
		Entity entity = Scene::CreateEntity(name);
		entity.GetComponents<IDComponent>().ID = { guid };
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
		// TODO: is there more to this cleanup?
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		// TODO: rewrite this, using:
		// https://github.com/skypjack/entt/discussions/684#discussion-3299774
		// https://github.com/skypjack/entt/issues/694

		std::string newTag = entity.GetName() + " copy";
		Entity copy = CreateEntity(newTag);

		CopyComponentIfExists<TransformComponent>		(entity, copy);
		CopyComponentIfExists<SpriteComponent>			(entity, copy);
		CopyComponentIfExists<CircleComponent>			(entity, copy);
		CopyComponentIfExists<CameraComponent>			(entity, copy);
		CopyComponentIfExists<RigidBody2DComponent>		(entity, copy);
		CopyComponentIfExists<RectColliderComponent>	(entity, copy);
		CopyComponentIfExists<CircleColliderComponent>	(entity, copy);
		CopyComponentIfExists<NativeScriptComponent>	(entity, copy);

		return copy;
	}

	void Scene::OnRuntimeStart()
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
		auto circleEntities = m_Registry.view<TransformComponent, CircleComponent>();

		Renderer::BeginScene(camera);

		for (auto entity : spriteEntities)
		{
			auto [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

			Renderer::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		}

		for (auto entity : circleEntities)
		{
			auto [transform, circle] = circleEntities.get<TransformComponent, CircleComponent>(entity);

			Renderer::DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entity);
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
				// TODO: this stuff should be in its own method, since it has an identical twin in OnUpdateEditor
				auto spriteEntities = m_Registry.view<TransformComponent, SpriteComponent>();
				auto circleEntities = m_Registry.view<TransformComponent, CircleComponent>();

				Renderer::BeginScene(activeCamera->GetProjection(), cameraTransform);

				for (auto entity : spriteEntities)
				{
					auto [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

					Renderer::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				}

				for (auto entity : circleEntities)
				{
					auto [transform, circle] = circleEntities.get<TransformComponent, CircleComponent>(entity);

					Renderer::DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entity);
				}

				// TODO: draw colliders, with appropriate offsets/scaling

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

			if (entity.HasComponents<CircleColliderComponent>())
			{
				auto& collider = entity.GetComponents<CircleColliderComponent>();

				b2CircleShape shape;
				shape.m_p.Set(collider.Offset.x, collider.Offset.y);
				shape.m_radius = collider.Radius;

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

}

