#pragma once

#include "Zahra/Core/Buffer.h"
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
		Bool,
		sByte,
		Byte,
		Short,
		uShort,
		Char,
		Int,
		uInt,
		Long,
		uLong,
		EntityID,
		Float,
		Double,

		// Djinn types
		Vector2,
		Vector3,
		Vector4,
	};

	namespace ScriptUtils
	{
		inline const char* GetScriptFieldTypeName(ScriptFieldType type)
		{
			switch (type)
			{
			case ScriptFieldType::Bool:			return "bool";
			case ScriptFieldType::sByte:		return "sbyte";
			case ScriptFieldType::Byte:			return "byte";
			case ScriptFieldType::Short:		return "short";
			case ScriptFieldType::uShort:		return "ushort";
			case ScriptFieldType::Char:			return "char";
			case ScriptFieldType::Int:			return "int";
			case ScriptFieldType::uInt:			return "uint";
			case ScriptFieldType::Long:			return "long";
			case ScriptFieldType::uLong:		return "ulong";
			case ScriptFieldType::Float:		return "float";
			case ScriptFieldType::Double:		return "double";
			case ScriptFieldType::EntityID:		return "EntityID";
			case ScriptFieldType::Vector2:		return "Vector2";
			case ScriptFieldType::Vector3:		return "Vector3";
			case ScriptFieldType::Vector4:		return "Vector4";
			}
			Z_CORE_ASSERT(false, "Unsupported ScriptFieldType");
			return "";
		}

		inline uint32_t GetScriptFieldTypeByteSize(ScriptFieldType type)
		{
			switch (type)
			{
				case ScriptFieldType::Bool:
				case ScriptFieldType::sByte:
				case ScriptFieldType::Byte:
					return 1;
				case ScriptFieldType::Short:
				case ScriptFieldType::uShort:
				case ScriptFieldType::Char: // C# chars are utf16
					return 2;
				case ScriptFieldType::Int:
				case ScriptFieldType::uInt:
				case ScriptFieldType::Float:
					return 4;
				case ScriptFieldType::Long:
				case ScriptFieldType::uLong:
				case ScriptFieldType::Double:
				case ScriptFieldType::EntityID:
				case ScriptFieldType::Vector2:
					return 8;
				case ScriptFieldType::Vector3:
					return 12;
				case ScriptFieldType::Vector4:
					return 16;
			}
			Z_CORE_ASSERT(false, "Unsupported ScriptFieldType");
			return 0;
		}
	}	

	struct ScriptField
	{
		ScriptFieldType Type = ScriptFieldType::None;
		std::string Name;
		MonoClassField* MonoField = nullptr;
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

		MonoObject* Instantiate();
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** args);

	private:
		MonoClass* m_MonoClass = nullptr;

		std::string m_FullClassName;

		std::vector<ScriptField> m_PublicFields;

		friend class ScriptInstance;
		friend class ScriptEngine;
	};

	class ScriptInstance : public RefCounted
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, ZGUID guid);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		template <typename T>
		T GetScriptFieldValue(const ScriptField& field)
		{
			T value;

			GetScriptFieldValue(m_MonoObject, field.MonoField, (void*)&value);

			return value;
		}

		template <typename T>
		void SetScriptFieldValue(const ScriptField& field, T& value)
		{
			SetScriptFieldValue(m_MonoObject, field.MonoField, (void*)&value);
		}

	protected:
		void InvokeOnCreate();
		void InvokeEarlyUpdate(float dt);
		void InvokeLateUpdate(float dt);

		MonoObject* GetMonoObject() const { return m_MonoObject; }

	private:
		Ref<ScriptClass> m_ScriptClass;
		MonoObject* m_MonoObject = nullptr;

		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreate = nullptr;
		MonoMethod* m_OnEarlyUpdate = nullptr; // pre-physics
		MonoMethod* m_OnLateUpdate = nullptr; // post-physics

		void GetScriptFieldValue(MonoObject* object, MonoClassField* field, void* destination);
		void SetScriptFieldValue(MonoObject* object, MonoClassField* field, void* source);

		friend class ScriptEngine;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void ReloadAssembly();

		static const std::unordered_map<std::string, Ref<ScriptClass>>& GetScriptClasses();
		static const Ref<ScriptClass> GetScriptClassIfValid(const std::string& fullName);
		static bool ValidScriptClass(const std::string& fullName);

		static void CreateScriptInstance(Entity entity);
		static void ScriptInstanceEarlyUpdate(Entity entity, float dt);
		static void ScriptInstanceLateUpdate(Entity entity, float dt);

		static Ref<ScriptInstance> GetScriptInstance(Entity entity);
		static MonoObject* GetMonoObject(ZGUID guid);

		static Entity GetEntity(ZGUID guid);
		static Entity GetEntity(MonoString* name);		

		static MonoString* StdStringToMonoString(const std::string& string);

	private:
		static void CreateRootDomain();
		static void CreateAppDomain();
		static void ShutdownMonoDomains();
		
		static void LoadAssemblies();
		static void LoadAssembly(const std::filesystem::path& library, MonoAssembly*& assembly, MonoImage*& assemblyImage);

		static void Reflect();
	};

}
