#include "zpch.h"
#include "Scene.h"

#include "Zahra/Renderer/Renderer.h"
#include "Zahra/Scene/Entity.h"
#include "Zahra/Scene/ScriptableEntity.h"
#include "Zahra/Scene/Components.h"
#include "Zahra/Scripting/ScriptEngine.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

namespace Zahra
{
	static Scene::OverlayMode s_OverlayMode;

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
		m_EntityMap.clear();
		m_Registry.clear();
	}

	template<typename... ComponentType>
	static void CopyComponent(entt::registry& srcReg, entt::registry& destReg, const std::unordered_map<ZGUID, entt::entity>& guidToNewHandle)
	{

		([&]()
			{
				auto componentView = destReg.view<ComponentType>();
				for (auto oldHandle : componentView)
				{
					ZGUID guid = destReg.get<IDComponent>(oldHandle).ID;
					entt::entity newHandle = guidToNewHandle.at(guid);

					auto& oldComponent = destReg.get<ComponentType>(oldHandle);
					srcReg.emplace_or_replace<ComponentType>(newHandle, oldComponent);
				}
			}
		(), ...);

	}

	template<typename... ComponentType>
	static void CopyComponent(ComponentGroup<ComponentType...>, entt::registry& srcReg, entt::registry& destReg, const std::unordered_map<ZGUID, entt::entity>& guidToNewHandle)
	{
		CopyComponent<ComponentType...>(srcReg, destReg, guidToNewHandle);
	}

	template <typename... ComponentType>
	static void CopyComponentIfExists(Entity srcEnt, Entity destEnt)
	{
		([&]()
			{
				if (!srcEnt.HasComponents<ComponentType>()) return;

				destEnt.AddOrReplaceComponent<ComponentType>(srcEnt.GetComponents<ComponentType>());
			}
		(), ...);
	}

	template<typename... ComponentType>
	static void CopyComponentIfExists(ComponentGroup<ComponentType...>, Entity srcReg, Entity destReg)
	{
		CopyComponentIfExists<ComponentType...>(srcReg, destReg);
	}

	Ref<Scene> Scene::CopyScene(Ref<Scene> oldScene)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->SetName(oldScene->GetName());

		newScene->m_ViewportWidth = oldScene->m_ViewportWidth;
		newScene->m_ViewportHeight = oldScene->m_ViewportHeight;

		auto& oldRegistry = oldScene->m_Registry;
		auto& newRegistry = newScene->m_Registry;

		std::unordered_map<ZGUID, entt::entity> guidToNewHandle;

		// copy entities, along with their IDComponents and TagComponents
		oldRegistry.view<entt::entity>().each([&](auto entityHandle)
			{
				Entity oldEntity = { entityHandle, oldScene.get() };
				ZGUID guid = oldEntity.GetGUID();
				guidToNewHandle[guid] = (entt::entity)newScene->CreateEntity(guid, oldEntity.GetComponents<TagComponent>().Tag);
			});

		// copy the remaing components
		CopyComponent(AllComponents{}, newRegistry, oldRegistry, guidToNewHandle);

		return newScene;
	}

	// All entities will automatically be created with an IDComponent, TagComponent and TransformComponent
	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.GetComponents<TagComponent>().Tag = name.empty() ? "unnamed_entity" : name;
		m_EntityMap[entity.GetGUID()] = entity;
		return entity;
	}

	Entity Scene::CreateEntity(uint64_t guid, const std::string& name)
	{
		Entity entity = Scene::CreateEntity(name);
		entity.GetComponents<IDComponent>().ID = { guid };
		m_EntityMap[entity.GetGUID()] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetGUID());
		m_Registry.destroy(entity);
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		// create new component with its own IDComponent and the copied TagComponent
		std::string newTag = entity.GetName();
		Entity copy = CreateEntity(newTag);

		// copy the remaining components
		CopyComponentIfExists(AllComponents{}, entity, copy);

		return copy;
	}

	Entity Scene::GetEntity(ZGUID guid)
	{
		Z_CORE_ASSERT(m_EntityMap.find(guid) != m_EntityMap.end());
		return { m_EntityMap.at(guid), this };
	}

	void Scene::OnRuntimeStart()
	{
		OnSimulationStart();

		// Initialise script behaviours
		{
			ScriptEngine::OnRuntimeStart(this);
			
			auto& view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				ScriptEngine::InstantiateScript(entity);
			}
		}
		
	}

	void Scene::OnRuntimeStop()
	{
		OnSimulationStop();

		ScriptEngine::OnRuntimeStop();

	}

	void Scene::OnSimulationStart()
	{
		InitPhysicsWorld();
	}

	void Scene::OnSimulationStop()
	{
		for (auto& body : m_PhysicsBodies) m_PhysicsWorld->DestroyBody(body.second);
		m_PhysicsBodies.clear();
		m_PhysicsWorld = nullptr;

		
	}

	void Scene::OnUpdateEditor(float dt, EditorCamera& camera)
	{
		Renderer::BeginScene(camera);
		RenderEntities();
		Renderer::EndScene();
	}

	void Scene::OnUpdateSimulation(float dt, EditorCamera& camera)
	{
		UpdatePhysicsWorld(dt);

		Renderer::BeginScene(camera);
		RenderEntities();
		Renderer::EndScene();
	}

	void Scene::OnUpdateRuntime(float dt)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RUN SCRIPTS
		{
			auto& view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				ScriptEngine::UpdateScript(entity, dt);
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RUN PHYSICS SIMULATION
		UpdatePhysicsWorld(dt);
		
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
				Renderer::BeginScene(activeCamera->GetProjection(), cameraTransform);
				RenderEntities();
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
				shape.SetAsBox(transformComp.Scale.x * collider.HalfExtent.x, transformComp.Scale.y * collider.HalfExtent.y, b2Vec2(collider.Offset.x, collider.Offset.y), .0f);

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
				// NOTE: ellipse collision handling (sounds horrendous) is not supported by box2d

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

	void Scene::UpdatePhysicsWorld(float dt)
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

	const Scene::OverlayMode& Scene::GetOverlayMode()
	{
		return s_OverlayMode;
	}

	void Scene::SetOverlayMode(Scene::OverlayMode mode)
	{
		s_OverlayMode = mode;
	}

	void Scene::RenderEntities()
	{
		auto spriteEntities = m_Registry.view<TransformComponent, SpriteComponent>();

		for (auto entity : spriteEntities)
		{
			auto [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

			Renderer::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		}

		auto circleEntities = m_Registry.view<TransformComponent, CircleComponent>();

		for (auto entity : circleEntities)
		{
			auto [transform, circle] = circleEntities.get<TransformComponent, CircleComponent>(entity);

			Renderer::DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entity);
		}

		
		if (s_OverlayMode.ShowColliders)
		{
			auto rectColliders = m_Registry.view<TransformComponent, RectColliderComponent>();

			for (auto entity : rectColliders)
			{
				auto [transform, collider] = rectColliders.get<TransformComponent, RectColliderComponent>(entity);

				glm::vec3 scale = transform.Scale * glm::vec3(collider.HalfExtent * 2.0f, 1.0f);

				glm::mat4 colliderTransform = glm::translate(glm::mat4(1.0f), glm::vec3(transform.Translation.x, transform.Translation.y, 0.f))
					* glm::rotate(glm::mat4(1.0f), transform.EulerAngles.z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(collider.Offset, 0.f))
					* glm::scale(glm::mat4(1.0f), scale);

				Renderer::DrawRect(colliderTransform, s_OverlayMode.ColliderColour);
			}

			auto circleColliders = m_Registry.view<TransformComponent, CircleColliderComponent>();

			for (auto entity : circleColliders)
			{
				auto [transform, collider] = circleColliders.get<TransformComponent, CircleColliderComponent>(entity);

				glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), transform.Translation)
					* glm::rotate(glm::mat4(1.0f), transform.EulerAngles.z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(collider.Offset, 0.f))
					* glm::scale(glm::mat4(1.f), glm::vec3(collider.Radius * 2.05f));

				Renderer::DrawCircle(colliderTransform, s_OverlayMode.ColliderColour, .02f / collider.Radius, .001f, (int)entity);
			}

		}

	}


}

