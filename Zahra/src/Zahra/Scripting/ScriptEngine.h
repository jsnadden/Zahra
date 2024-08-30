#pragma once

#include "Zahra/Scene/Entity.h"

namespace Zahra
{
	class ScriptClass;
	class Scene;

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();
		static void OnRuntimeUpdate();

		static void InstantiateScript(Entity entity);
		static void UpdateScript(Entity entity, float dt);

		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityTypes();
		static bool ValidEntityClass(const std::string& fullName);

	private:
		static void InitMonoDomains();
		static void LoadCoreAssembly(std::filesystem::path filepath);
		static void ShutdownMonoDomains();

		static void ReflectAssemblyTypes();
	};

}
