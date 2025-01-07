#pragma once

#include "Zahra/Scene/Entity.h"
#include "Zahra/Scripting/MonoExterns.h"

// TODO: replace mono with something better documented XD
// also maybe refactor to use numerical IDs for classes/instances/fields, instead of strings

namespace Zahra
{
	enum class ScriptFieldType
	{
		None,

		// Simple types		
		sByte,
		Byte,
		Short,
		uShort,
		Int,
		uInt,
		Long,
		uLong,

		Float,
		Double,

		Char,
		Bool,

		// Djinn custom types
		Entity,

		Vector2,
		Vector3,
		Vector4,
		Quaternion,
	};

	struct ScriptField
	{
		ScriptFieldType Type = ScriptFieldType::None;
		std::string Name;
	};

	class ScriptClass : public RefCounted
	{
	public:
		ScriptClass() = default;
		ScriptClass(const ScriptClass& other);

		bool IsSubclassOf(ScriptClass& other, bool checkInterfaces = false);

		const std::string& GetName() { return m_FullClassName; }
		const std::vector<ScriptField>& GetPublicFields() const { return m_PublicFields; }

	protected:
		ScriptClass(MonoImage* image, const std::string& classNamespace, const std::string& className);

		MonoClass* GetMonoClass() { return m_MonoClass; }
		MonoMethod* GetMethod(const std::string& methodName, int numArgs);
		const std::unordered_map<std::string, MonoClassField*>& GetMonoFields() { return m_MonoFields; }

		MonoObject* Instantiate();
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** args);

		void ReflectFields();
		//void ReflectMethods();

	private:
		MonoClass* m_MonoClass = nullptr;

		std::string m_FullClassName;

		std::vector<ScriptField> m_PublicFields;
		std::unordered_map<std::string, MonoClassField*> m_MonoFields;

		friend class ScriptInstance;
		friend class ScriptEngine;
	};

	class ScriptInstance : public RefCounted
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, ZGUID guid);

		Ref<ScriptClass> GetEntityClass() { return m_EntityClass; }

		void GetScriptFieldValue(const std::string& fieldName, void* destination);
		void SetScriptFieldValue(const std::string& fieldName, void* source);

	protected:
		void InvokeOnCreate();
		void InvokeEarlyUpdate(float dt);
		void InvokeLateUpdate(float dt);

		MonoObject* GetMonoObject() const { return m_MonoObject; }

	private:
		Ref<ScriptClass> m_EntityClass;
		MonoObject* m_MonoObject = nullptr;

		MonoMethod* m_ConstructFromGUID = nullptr;
		MonoMethod* m_OnCreate = nullptr;
		MonoMethod* m_OnEarlyUpdate = nullptr; // pre-physics
		MonoMethod* m_OnLateUpdate = nullptr; // post-physics

		friend class ScriptEngine;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void CreateScriptInstance(Entity entity);
		static void ScriptInstanceEarlyUpdate(Entity entity, float dt);
		static void ScriptInstanceLateUpdate(Entity entity, float dt);

		static const std::unordered_map<std::string, Ref<ScriptClass>>& GetScriptClasses();
		static bool ValidScriptClass(const std::string& fullName);

		static Ref<ScriptInstance> GetScriptInstance(Entity entity);

		static Entity GetEntity(ZGUID guid);

		static MonoString* StdStringToMonoString(const std::string& string);

	private:
		static void InitMonoDomains();
		static void ShutdownMonoDomains();
		
		static void LoadAssembly(const std::filesystem::path& library, MonoAssembly*& assembly, MonoImage*& assemblyImage);

		static void ReflectScriptClasses();

		//friend class ScriptGlue;
	};

}
