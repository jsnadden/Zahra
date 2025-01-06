#pragma once

#include "Zahra/Scene/Entity.h"

// TODO: replace mono with something better documented XD

extern "C"
{
	typedef struct _MonoAssembly		MonoAssembly;
	typedef struct _MonoClass			MonoClass;
	typedef struct _MonoClassField		MonoClassField;
	typedef struct _MonoImage			MonoImage;
	typedef struct _MonoMethod			MonoMethod;
	typedef struct _MonoObject			MonoObject;
	typedef struct _MonoString			MonoString;
}

namespace Zahra
{
	enum class ScriptFieldType
	{
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
		std::string Name;
		ScriptFieldType Type;
	};

	class ScriptEntityType : public RefCounted
	{
	public:
		ScriptEntityType() = default;
		ScriptEntityType(const ScriptEntityType& other);
		ScriptEntityType(MonoImage* image, const std::string& classNamespace, const std::string& className);

		MonoObject* Instantiate();

		MonoMethod* GetMethod(const std::string& methodName, int numArgs);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** args);

		void ReflectFields();

		bool IsSubclassOf(ScriptEntityType& other, bool checkInterfaces = false);

		MonoClass* GetMonoClass() { return m_Class; }
		const std::string& GetNamespace() { return m_Namespace; }
		const std::string& GetName() { return m_Name; }

	private:
		MonoClass* m_Class = nullptr;

		std::string m_Namespace;
		std::string m_Name;

		std::vector<ScriptField> m_PublicFields;
		std::vector<MonoClassField*> m_MonoFields;
	};

	class ScriptEntityInstance : public RefCounted
	{
	public:
		ScriptEntityInstance(Ref<ScriptEntityType> scriptClass, ZGUID guid);

		void InvokeOnCreate();
		void InvokeOnUpdate(float dt);

	private:
		Ref<ScriptEntityType> m_Class;
		MonoObject* m_Object = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreate = nullptr;
		MonoMethod* m_OnUpdate = nullptr;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void InstantiateScript(Entity entity);
		static void UpdateScript(Entity entity, float dt);

		static std::unordered_map<std::string, Ref<ScriptEntityType>> GetEntityTypes();
		static bool ValidEntityClass(const std::string& fullName);

		static Entity GetEntity(ZGUID guid);

		static MonoString* GetMonoString(const std::string& string);

	private:
		static void InitMonoDomains();
		static void ShutdownMonoDomains();
		
		static void LoadAssembly(const std::filesystem::path& library, MonoAssembly*& assembly, MonoImage*& assemblyImage);

		static void ReflectScriptEntities();

		friend class ScriptGlue;
	};

}
