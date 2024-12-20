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

	class Scene : public RefCounted
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> CopyScene(Ref<Scene> oldScene);

		Entity CreateEntity(const std::string& name = "unnamed_entity");
		Entity CreateEntity(uint64_t guid, const std::string& name = "unnamed_entity");
		void DestroyEntity(Entity entity);
		Entity DuplicateEntity(Entity entity);
		Entity GetEntity(ZGUID guid);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateEditor(float dt, EditorCamera& camera);
		void OnUpdateSimulation(float dt, EditorCamera& camera);
		void OnUpdateRuntime(float dt);

		// TODO: replace Box2D with a 3d physics engine (e.g. Nvidia PhysX)
		void InitPhysicsWorld();
		void UpdatePhysicsWorld(float dt);

		void OnViewportResize(float width, float height);

		const std::string& GetName() { return m_SceneName; }
		void SetName(const std::string& name) { m_SceneName = name; }

		void SetActiveCamera(Entity entity);
		Entity GetActiveCamera();

		void InitCameraComponent(entt::basic_registry<entt::entity>& registry, entt::entity entity)
		{
			Z_CORE_ASSERT(m_Registry.valid(entity), "Entity does not belong to this scene");

			if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
				m_Registry.get<CameraComponent>(entity).Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}

		struct OverlayMode
		{
			bool ShowColliders = false;
			glm::vec4 ColliderColour = { 0.80f, 0.55f, 0.00f, 1.00f };

			OverlayMode() = default;
			OverlayMode(const OverlayMode&) = default;
		};

		static const OverlayMode& GetOverlayMode();
		static void SetOverlayMode(OverlayMode mode);

	private:
		std::string m_SceneName;

		std::unique_ptr<b2World>(m_PhysicsWorld);
		std::map<entt::entity, b2Body*> m_PhysicsBodies;

		entt::basic_registry<entt::entity> m_Registry;
		std::unordered_map<ZGUID, entt::entity> m_EntityMap;

		entt::entity m_ActiveCamera = entt::null;

		float m_ViewportWidth = 1.0f, m_ViewportHeight = 1.0f;

		void RenderEntities();

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class PropertiesPanel;
		friend class SceneSerialiser;
	};

}


