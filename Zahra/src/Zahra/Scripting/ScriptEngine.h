#pragma once

#include "Zahra/Scene/Entity.h"

namespace Zahra
{
	class EntityScriptType;
	class Scene;

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void InstantiateScript(Entity entity);
		static void UpdateScript(Entity entity, float dt);

		static std::unordered_map<std::string, Ref<EntityScriptType>> GetEntityTypes();
		static bool ValidEntityClass(const std::string& fullName);

		static Entity GetEntity(ZGUID guid);

	private:
		static void InitMonoDomains();
		static void LoadCoreAssembly(std::filesystem::path filepath);
		static void ShutdownMonoDomains();

		static void ReflectAssemblyTypes();
	};

}
