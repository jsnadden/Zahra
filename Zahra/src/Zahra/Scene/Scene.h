#pragma once

#include "Components.h"
#include "Zahra/Renderer/EditorCamera.h"

#include <entt.hpp>

// forward declare Box2D classes
class b2World;
class b2Body;

namespace Zahra
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> CopyScene(Ref<Scene> oldScene);

		Entity CreateEntity(const std::string& name = "unnamed_entity");
		Entity CreateEntity(uint64_t guid, const std::string& name = "unnamed_entity");
		void DestroyEntity(Entity entity);
		Entity DuplicateEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateEditor(float dt, EditorCamera& camera);
		void OnUpdateRuntime(float dt);

		// TODO: replace Box2D with a 3d physics engine (e.g. Nvidia PhysX)
		void InitPhysicsWorld();
		void UpdatePhysicsWorld();

		void OnViewportResize(float width, float height);

		const std::string& GetName() { return m_SceneName; }
		void SetName(const std::string& name) { m_SceneName = name; }

		Entity GetActiveCamera();

		void InitCameraComponent(entt::basic_registry<entt::entity>& registry, entt::entity entity)
		{
			Z_CORE_ASSERT(m_Registry.valid(entity), "Entity does not belong to this scene");
			
			if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
				m_Registry.get<CameraComponent>(entity).Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}

	private:
		std::string m_SceneName;
		
		std::unique_ptr<b2World>(m_PhysicsWorld);
		std::map<entt::entity, b2Body*> m_PhysicsBodies; // TODO: replace entt::entity with internal uuids?

		entt::basic_registry<entt::entity> m_Registry; // TODO: custom UUIDs (change the template param)
		float m_ViewportWidth = 1.0f, m_ViewportHeight = 1.0f;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class PropertiesPanel;
		friend class SceneSerialiser;
	};

}


