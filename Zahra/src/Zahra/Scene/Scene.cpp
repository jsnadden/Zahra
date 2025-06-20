#include "zpch.h"
#include "Scene.h"

#include "Zahra/Assets/AssetManager.h"
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
	static Scene::DebugRenderSettings s_DebugRenderSettings;

	Scene::Scene(const std::string& sceneName)
	{
		m_SceneName = sceneName;

		// connect entt callback signals
		m_Registry.on_construct<entt::entity>().connect<&entt::registry::emplace_or_replace<IDComponent>>();
		m_Registry.on_construct<entt::entity>().connect<&entt::registry::emplace_or_replace<TagComponent>>();
		m_Registry.on_construct<entt::entity>().connect<&entt::registry::emplace_or_replace<TransformComponent>>();

		m_Registry.on_construct<CameraComponent>().connect<&Scene::InitCameraComponentViewportSize>(this);
		m_Registry.on_destroy<CameraComponent>().connect<&Scene::DeactivateCamera>(this);

		m_Registry.on_construct<ScriptComponent>().connect<&Scene::AllocateScriptComponentFieldStorage>(this);
		m_Registry.on_update<ScriptComponent>().connect<&Scene::AllocateScriptComponentFieldStorage>(this);
	}

	Scene::~Scene()
	{
		m_EntityMap.clear();
		m_Registry.clear();

		for (auto& [uuid, buffer] : m_ScriptFieldStorage)
			buffer.Release();
		m_ScriptFieldStorage.clear();
	}

	template<typename... ComponentType>
	static void CopyComponent(entt::registry& srcReg, entt::registry& destReg, const std::unordered_map<UUID, entt::entity>& uuidToNewHandle)
	{
		([&]()
			{
				auto componentView = srcReg.view<ComponentType>();
				for (auto oldHandle : componentView)
				{
					UUID uuid = srcReg.get<IDComponent>(oldHandle).ID;
					entt::entity newHandle = uuidToNewHandle.at(uuid);

					auto& oldComponent = srcReg.get<ComponentType>(oldHandle);
					destReg.emplace_or_replace<ComponentType>(newHandle, oldComponent);
				}
			}
		(), ...);
	}

	template<typename... ComponentType>
	static void CopyComponent(ComponentGroup<ComponentType...>, entt::registry& srcReg, entt::registry& destReg, const std::unordered_map<UUID, entt::entity>& uuidToNewHandle)
	{
		CopyComponent<ComponentType...>(srcReg, destReg, uuidToNewHandle);
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

		std::unordered_map<UUID, entt::entity> uuidToNewHandle;

		// copy entities, along with their IDComponents and TagComponents
		srcRegistry.view<entt::entity>().each([&](auto entityHandle)
			{
				Entity oldEntity = { entityHandle, srcScene.Raw() };
				UUID uuid = oldEntity.GetID();
				Entity newEntity = destScene->CreateEntity(uuid, oldEntity.GetComponents<TagComponent>().Tag);
				uuidToNewHandle[uuid] = (entt::entity)newEntity;
			});

		// copy the remaining components
		CopyComponent(MostComponents{}, srcRegistry, destRegistry, uuidToNewHandle);

		// set active camera
		Entity oldCamera = srcScene->GetActiveCamera();
		if (oldCamera) destScene->SetActiveCamera({ uuidToNewHandle[oldCamera.GetID()] , destScene.Raw() });

		// copy field storage buffer
		for (auto& [uuid, srcBuffer] : srcScene->m_ScriptFieldStorage)
		{
			auto& destBuffer = destScene->m_ScriptFieldStorage[uuid];
			destBuffer.Release();
			destBuffer = Buffer::Copy(srcBuffer);
		}

		return destScene;
	}

	// All entities will automatically be created with an IDComponent, TagComponent and TransformComponent
	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.GetComponents<TagComponent>().Tag = name;
		m_EntityMap[entity.GetID()] = entity;
		return entity;
	}

	Entity Scene::CreateEntity(uint64_t uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.GetComponents<TagComponent>().Tag = name;
		entity.GetComponents<IDComponent>().ID = { uuid };
		m_EntityMap[uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if ((entt::entity)entity == m_ActiveCamera)
			m_ActiveCamera = entt::null;

		m_EntityMap.erase(entity.GetID());
		m_Registry.destroy(entity);
	}

	void Scene::DestroyEntity(UUID uuid)
	{
		auto it = m_EntityMap.find(uuid);
		if (it == m_EntityMap.end())
			return;

		auto entity = it->second;
		m_EntityMap.erase(uuid);
		m_Registry.destroy(entity);
	}

	Entity Scene::DuplicateEntity(Entity extantEntity, UUID newID)
	{
		// create new component with its own IDComponent and the copied TagComponent
		std::string newName = extantEntity.GetName();
		Entity newEntity = CreateEntity(newID, newName);

		// copy the remaining components
		CopyComponentIfExists(MostComponents{}, extantEntity, newEntity);

		return newEntity;
	}

	Entity Scene::GetEntity(UUID uuid)
	{
		auto it = m_EntityMap.find(uuid);
		if (it == m_EntityMap.end())
			return { entt::null, this };

		return { it->second, this };
	}

	void Scene::ForEachEntity(const std::function<void(Entity entity)>& action)
	{
		// TODO: add another std::function argument to set iteration order
		// (could also make this templated for ordering via component values)
		//m_Registry.sort<entt::entity>([](const auto& lhs, const auto& rhs) { return lhs < rhs; });

		m_Registry.view<entt::entity>().each([&](auto entityHandle)
			{
				Entity entity = { entityHandle, this };
				action(entity);
			});
	}

	Entity Scene::GetEntity(const std::string_view& name)
	{
		auto view = m_Registry.view<entt::entity>();
		for (auto e : view)
		{
			Entity entity = { e, this };

			if (name == entity.GetComponents<TagComponent>().Tag)
				return entity;
		}

		return { entt::null, this };
	}

	void Scene::InitCameraComponentViewportSize(entt::basic_registry<entt::entity>& registry, entt::entity e)
	{
		Z_CORE_ASSERT(m_Registry.valid(e), "Entity does not belong to this scene");

		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			m_Registry.get<CameraComponent>(e).Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	void Scene::DeactivateCamera(entt::basic_registry<entt::entity>& registry, entt::entity e)
	{
		Z_CORE_ASSERT(m_Registry.valid(e), "Entity does not belong to this scene");

		if (m_ActiveCamera == e)
			m_ActiveCamera == entt::null;
	}

	void Scene::AllocateScriptComponentFieldStorage(entt::basic_registry<entt::entity>& registry, entt::entity e)
	{
		Z_CORE_ASSERT(m_Registry.valid(e), "Entity does not belong to this scene");

		AllocateScriptFieldStorage({ e, this });
	}

	//void Scene::FreeScriptComponentFieldStorage(entt::basic_registry<entt::entity>& registry, entt::entity e)
	//{
	//	Z_CORE_ASSERT(m_Registry.valid(e), "Entity does not belong to this scene");

	//	Entity entity = { e, this };
	//	Z_CORE_ASSERT(entity.HasComponents<IDComponent>(), "Freeing field storage requires a uuid for buffer lookup");

	//	FreeScriptFieldStorage(entity);
	//}

	//void Scene::DestroyScriptComponentBeforeIDComponent(entt::basic_registry<entt::entity>& registry, entt::entity e)
	//{
	//	// must remove ScriptComponent before IDComponent
	//	Entity entity = { e, this };
	//	if (entity.HasComponents<ScriptComponent>())
	//		entity.RemoveComponent<ScriptComponent>();
	//}

	void Scene::OnRuntimeStart()
	{
		Z_CORE_INFO("Scene '{}' has begun runtime", m_SceneName);

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

		Z_CORE_INFO("Scene '{}' has ended runtime", m_SceneName);
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

	void Scene::OnRenderEditor(Ref<Renderer2D> renderer, const EditorCamera& camera, Entity selection, const glm::vec4& highlightColour)
	{
		renderer->ResetStats();

		if (s_DebugRenderSettings.LineWidth > 0.0f)
			renderer->SetLineWidth(s_DebugRenderSettings.LineWidth);

		renderer->BeginScene(camera);
		{
			RenderEntities(renderer);
			RenderDebug(renderer, selection, highlightColour);
		}
		renderer->EndScene();
	}

	void Scene::OnRenderRuntime(Ref<Renderer2D> renderer, Entity selection, const glm::vec4& highlightColour)
	{
		if (m_ActiveCamera != entt::null)
		{
			Entity activeCameraEntity(m_ActiveCamera, this);
			glm::mat4 cameraView = glm::inverse(activeCameraEntity.GetComponents<TransformComponent>().GetTransform());
			glm::mat4 cameraProjection = activeCameraEntity.GetComponents<CameraComponent>().Camera.GetProjection();

			renderer->ResetStats();

			if (s_DebugRenderSettings.LineWidth > 0.0f)
				renderer->SetLineWidth(s_DebugRenderSettings.LineWidth);

			renderer->BeginScene(cameraView, cameraProjection);
			{
				RenderEntities(renderer);
				RenderDebug(renderer, selection, highlightColour);
			}
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
		return b2BodyType::b2_staticBody;
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
			InitCameraComponentViewportSize(m_Registry, entity);
		else
			entity.AddComponent<CameraComponent>();

		m_ActiveCamera = entity;
	}

	Entity Scene::GetActiveCamera()
	{
		Entity camera =  { m_ActiveCamera, this };
		if (camera.HasComponents<CameraComponent>())
		{
			return camera;
		}
		else
		{
			m_ActiveCamera = entt::null;
			return { entt::null, this };
		}
	}

	void Scene::AllocateScriptFieldStorage(Entity entity)
	{
		auto& component = entity.GetComponents<ScriptComponent>();

		if (auto scriptClass = ScriptEngine::GetScriptClassIfValid(component.ScriptName))
		{
			uint64_t fieldCount = scriptClass->GetPublicFields().size();
			auto& buffer = m_ScriptFieldStorage[entity.GetID()];

			if (buffer.GetSize() != 16 * fieldCount)
			{
				buffer.Allocate(16 * fieldCount);
				buffer.ZeroInitialise();
			}
		}
	}

	Buffer Scene::GetScriptFieldStorage(Entity entity)
	{
		auto result = m_ScriptFieldStorage.find(entity.GetID());
		Z_CORE_ASSERT(result != m_ScriptFieldStorage.end());

		return result->second;
	}

	Scene::DebugRenderSettings& Scene::GetDebugRenderSettings()
	{
		return s_DebugRenderSettings;
	}

	void Scene::RenderEntities(Ref<Renderer2D>& renderer)
	{
		auto spriteEntities = m_Registry.view<TransformComponent, SpriteComponent>();
		for (auto entity : spriteEntities)
		{
			auto [transform, sprite] = spriteEntities.get<TransformComponent, SpriteComponent>(entity);

			if (sprite.TextureHandle)
			{
				Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(sprite.TextureHandle);
				renderer->DrawQuad(transform.GetTransform(), texture, sprite.Tint, sprite.TextureTiling, (int)entity);
			}
			else
			{
				renderer->DrawQuad(transform.GetTransform(), sprite.Tint, (int)entity);
			}
		}

		auto circleEntities = m_Registry.view<TransformComponent, CircleComponent>();
		for (auto entity : circleEntities)
		{
			auto [transform, circle] = circleEntities.get<TransformComponent, CircleComponent>(entity);

			renderer->DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entity);
		}
	}

	void Scene::RenderDebug(Ref<Renderer2D>& renderer, Entity selection, const glm::vec4& selectionColour)
	{
		if (s_DebugRenderSettings.ShowColliders)
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

				renderer->DrawQuadBoundingBox(colliderTransform, s_DebugRenderSettings.ColliderColour, (int)entity);
			}

			auto circleColliders = m_Registry.view<TransformComponent, CircleColliderComponent>();
			for (auto entity : circleColliders)
			{
				auto [transform, collider] = circleColliders.get<TransformComponent, CircleColliderComponent>(entity);

				glm::mat4 colliderTransform = glm::translate(glm::mat4(1.f), glm::vec3(transform.Translation.x, transform.Translation.y, 0.f))
					* glm::rotate(glm::mat4(1.0f), transform.GetEulers().z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(collider.Offset, 0.f))
					* glm::scale(glm::mat4(1.f), glm::vec3(collider.Radius * 2.0f));

				renderer->DrawCircle(colliderTransform, s_DebugRenderSettings.ColliderColour, .02f / collider.Radius, .001f, (int)entity);
			}

		}

		if (selection)
		{
			TransformComponent entityTransform = selection.GetComponents<TransformComponent>();

			// expand selection box
			glm::vec3 pushOut =
			{ 
				1.0f + s_DebugRenderSettings.SelectionPushOut / entityTransform.Scale.x,
				1.0f + s_DebugRenderSettings.SelectionPushOut / entityTransform.Scale.y,
				1.0f
			};

			renderer->DrawQuadBoundingBox(entityTransform.GetTransform(), selectionColour, selection, pushOut);
		}

	}


}

