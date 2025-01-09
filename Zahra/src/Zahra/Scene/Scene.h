#pragma once

#include "Zahra/Renderer/Cameras/EditorCamera.h"
#include "Zahra/Renderer/Renderer2D.h"
#include "Zahra/Scene/Components.h"

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
		Scene(const std::string& sceneName = "Untitled");
		~Scene();

		static Ref<Scene> CopyScene(Ref<Scene> oldScene);

		Entity CreateEntity(const std::string& name = "unnamed_entity");
		Entity CreateEntity(uint64_t guid, const std::string& name = "unnamed_entity");
		void DestroyEntity(Entity entity);
		Entity DuplicateEntity(Entity entity);
		Entity GetEntity(ZGUID guid);

		// entt signal callbacks
		void InitCameraComponentViewportSize(entt::basic_registry<entt::entity>& registry, entt::entity e);
		void UpdateScriptComponentFieldStorage(entt::basic_registry<entt::entity>& registry, entt::entity e);
		void FreeScriptComponentFieldStorage(entt::basic_registry<entt::entity>& registry, entt::entity e);
		void DestroyScriptComponentBeforeIDComponent(entt::basic_registry<entt::entity>& registry, entt::entity e);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateEditor(float dt);
		void OnUpdateSimulation(float dt);
		void OnUpdateRuntime(float dt);

		void OnRenderEditor(Ref<Renderer2D> renderer, EditorCamera& camera, Entity selection, const glm::vec4 highlightColour);
		void OnRenderRuntime(Ref<Renderer2D> renderer, Entity selection, const glm::vec4 highlightColour);

		// TODO: replace Box2D with a 3d physics engine (e.g. Nvidia PhysX)
		void InitPhysicsWorld();
		void UpdatePhysicsWorld(float dt);

		void OnViewportResize(float width, float height);

		const std::string& GetName() { return m_SceneName; }
		void SetName(const std::string& name) { m_SceneName = name; }

		void SetActiveCamera(Entity entity);
		Entity GetActiveCamera();

		struct DebugRenderSettings
		{
			bool ShowColliders = false;
			glm::vec4 ColliderColour = { 0.80f, 0.55f, 0.00f, 1.00f };
			float LineWidth = 3.0f;
		};

		static DebugRenderSettings& GetDebugRenderSettings();

	private:
		std::string m_SceneName;

		std::unique_ptr<b2World>(m_PhysicsWorld);
		//std::map<entt::entity, b2Body*> m_PhysicsBodies;

		entt::basic_registry<entt::entity> m_Registry;
		std::unordered_map<ZGUID, entt::entity> m_EntityMap;

		entt::entity m_ActiveCamera = entt::null;

		float m_ViewportWidth = 1.0f, m_ViewportHeight = 1.0f;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class PropertiesPanel;
		friend class SceneSerialiser;

		// TODO: move these to SceneRenderer
		void RenderEntities(Ref<Renderer2D>& renderer);
		void RenderDebug(Ref<Renderer2D>& renderer, Entity selection, const glm::vec4& highlightColour);
	};

}


