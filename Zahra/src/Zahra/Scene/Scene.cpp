#include "zpch.h"
#include "Scene.h"

#include "Zahra/Renderer/Renderer.h"
#include "Zahra/Scene/Components.h"
#include "Zahra/Scene/Entity.h"
#include "Zahra/Scene/ScriptableEntity.h"
#include "Zahra/Scripting/ScriptEngine.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

namespace Zahra
{
	static Scene::DebugRenderSettings s_OverlayMode;

	Scene::Scene(const std::string& sceneName)
	{
		m_SceneName = sceneName;

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
				auto componentView = srcReg.view<ComponentType>();
				for (auto oldHandle : componentView)
				{
					ZGUID guid = srcReg.get<IDComponent>(oldHandle).ID;
					entt::entity newHandle = guidToNewHandle.at(guid);

					auto& oldComponent = srcReg.get<ComponentType>(oldHandle);
					destReg.emplace_or_replace<ComponentType>(newHandle, oldComponent);
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
	static void CopyComponentIfExists(ComponentGroup<ComponentType...>, Entity srcEnt, Entity destEnt)
	{
		CopyComponentIfExists<ComponentType...>(srcEnt, destEnt);
	}

	Ref<Scene> Scene::CopyScene(Ref<Scene> srcScene)
	{
		Ref<Scene> destScene = Ref<Scene>::Create();

		destScene->SetName(srcScene->GetName());

		destScene->m_ViewportWidth = srcScene->m_ViewportWidth;
		destScene->m_ViewportHeight = srcScene->m_ViewportHeight;

		auto& srcRegistry = srcScene->m_Registry;
		auto& destRegistry = destScene->m_Registry;

		std::unordered_map<ZGUID, entt::entity> guidToNewHandle;

		// copy entities, along with their IDComponents and TagComponents
		srcRegistry.view<entt::entity>().each([&](auto entityHandle)
			{
				Entity oldEntity = { entityHandle, srcScene.Raw() };
				ZGUID guid = oldEntity.GetGUID();
				Entity newEntity = destScene->CreateEntity(guid, oldEntity.GetComponents<TagComponent>().Tag);
				guidToNewHandle[guid] = (entt::entity)newEntity;
			});

		// copy the remaing components
		CopyComponent(MostComponents{}, srcRegistry, destRegistry, guidToNewHandle);

		// set active camera
		Entity oldCamera = srcScene->GetActiveCamera();
		if (oldCamera) destScene->SetActiveCamera({ guidToNewHandle[oldCamera.GetGUID()] , destScene.Raw() });

		return destScene;
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
		CopyComponentIfExists(MostComponents{}, entity, copy);

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
				ScriptEngine::CreateScriptInstance(entity);
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
		auto view = m_Registry.view<RigidBody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& bodyComp = entity.GetComponents<RigidBody2DComponent>();
			m_PhysicsWorld->DestroyBody((b2Body*)bodyComp.RuntimeBody);
			bodyComp.RuntimeBody = nullptr;
		}

		m_PhysicsWorld = nullptr;
	}

	void Scene::OnUpdateEditor(float dt)
	{
		
	}

	void Scene::OnUpdateSimulation(float dt)
	{
		UpdatePhysicsWorld(dt);
	}

	void Scene::OnUpdateRuntime(float dt)
	{
		auto& view = m_Registry.view<ScriptComponent>();

		for (auto e : view)
		{
			Entity entity = { e, this };
			ScriptEngine::ScriptInstanceEarlyUpdate(entity, dt);
		}

		UpdatePhysicsWorld(dt);

		for (auto e : view)
		{
			Entity entity = { e, this };
			ScriptEngine::ScriptInstanceLateUpdate(entity, dt);
		}
	}

	void Scene::OnRenderEditor(Ref<Renderer2D> renderer, EditorCamera& camera, Entity selection, const glm::vec4 highlightColour)
	{
		renderer->BeginScene(camera);
		RenderEntities(renderer);
		RenderSelection(renderer, selection, highlightColour);
		renderer->EndScene();
	}

	void Scene::OnRenderRuntime(Ref<Renderer2D> renderer, Entity selection, const glm::vec4 highlightColour)
	{
		if (m_ActiveCamera != entt::null)
		{
			Entity activeCameraEntity(m_ActiveCamera, this);
			glm::mat4 cameraView = glm::inverse(activeCameraEntity.GetComponents<TransformComponent>().GetTransform());
			glm::mat4 cameraProjection = activeCameraEntity.GetComponents<CameraComponent>().Camera.GetProjection();

			renderer->BeginScene(cameraView, cameraProjection);
			RenderEntities(renderer);
			RenderSelection(renderer, selection, highlightColour);
			renderer->EndScene();
		}
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
		b2Vec2 gravity = { .0f, -9.8f };
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
			bodyDef.angle = transformComp.GetEulers().z;

			auto physicsBody = m_PhysicsWorld->CreateBody(&bodyDef);
			bodyComp.RuntimeBody = (void*)physicsBody;
			physicsBody->SetFixedRotation(bodyComp.FixedRotation);

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

				physicsBody->CreateFixture(&fixtureDef);
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

				physicsBody->CreateFixture(&fixtureDef);
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
			auto& tc = entity.GetComponents<TransformComponent>();
			auto& bc = entity.GetComponents<RigidBody2DComponent>();

			auto physicsBody = (b2Body*)bc.RuntimeBody;

			const auto& position = physicsBody->GetPosition();
			const auto& rotation = physicsBody->GetAngle();

			tc.Translation.x = position.x;
			tc.Translation.y = position.y;

			auto eulers = tc.GetEulers();
			tc.SetRotation({ eulers.x, eulers.y, rotation });
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

	void Scene::SetActiveCamera(Entity entity)
	{
		Z_CORE_ASSERT(m_Registry.valid(entity), "Entity does not belong to this scene");

		if (entity.HasComponents<CameraComponent>())
			InitCameraComponent(m_Registry, entity);
		else
			entity.AddComponent<CameraComponent>();

		m_ActiveCamera = entity;
	}

	Entity Scene::GetActiveCamera()
	{
		return { m_ActiveCamera, this };
	}

	Scene::DebugRenderSettings& Scene::GetDebugRenderSettings()
	{
		return s_OverlayMode;
	}

	void Scene::RenderEntities(Ref<Renderer2D>& renderer)
	{
		auto spriteEntities = m_Registry.view<TransformComponent, SpriteComponent>();
		for (auto entity : spriteEntities)
		{
			auto [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

			if (sprite.Texture)
				renderer->DrawQuad(transform.GetTransform(), sprite.Texture, sprite.Tint, sprite.TextureTiling, (int)entity);
			else
				renderer->DrawQuad(transform.GetTransform(), sprite.Tint, (int)entity);
		}

		auto circleEntities = m_Registry.view<TransformComponent, CircleComponent>();
		for (auto entity : circleEntities)
		{
			auto [transform, circle] = circleEntities.get<TransformComponent, CircleComponent>(entity);

			renderer->DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entity);
		}

		if (s_OverlayMode.ShowColliders)
		{
			auto rectColliders = m_Registry.view<TransformComponent, RectColliderComponent>();
			for (auto entity : rectColliders)
			{
				auto [transform, collider] = rectColliders.get<TransformComponent, RectColliderComponent>(entity);

				glm::vec3 scale = transform.Scale * glm::vec3(collider.HalfExtent * 2.0f, 1.0f);

				glm::mat4 colliderTransform = glm::translate(glm::mat4(1.0f), glm::vec3(transform.Translation.x, transform.Translation.y, 0.f))
					* glm::rotate(glm::mat4(1.0f), transform.GetEulers().z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(collider.Offset, 0.f))
					* glm::scale(glm::mat4(1.0f), scale);

				renderer->DrawQuadBoundingBox(colliderTransform, s_OverlayMode.ColliderColour, (int)entity);
			}

			auto circleColliders = m_Registry.view<TransformComponent, CircleColliderComponent>();
			for (auto entity : circleColliders)
			{
				auto [transform, collider] = circleColliders.get<TransformComponent, CircleColliderComponent>(entity);

				glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), transform.Translation)
					* glm::rotate(glm::mat4(1.0f), transform.GetEulers().z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(collider.Offset, 0.f))
					* glm::scale(glm::mat4(1.f), glm::vec3(collider.Radius * 2.05f));

				renderer->DrawCircle(colliderTransform, s_OverlayMode.ColliderColour, .02f / collider.Radius, .001f, (int)entity);
			}

		}
	}

	void Scene::RenderSelection(Ref<Renderer2D>& renderer, Entity selection, const glm::vec4& highlightColour)
	{
		if (!selection)
			return;

		TransformComponent entityTransform = selection.GetComponents<TransformComponent>();
		renderer->DrawQuadBoundingBox(entityTransform.GetTransform(), highlightColour);
		
	}


}

