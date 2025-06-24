#include "zpch.h"
#include "ScriptGlue.h"

#include "Zahra/Core/Application.h"
#include "Zahra/Scene/Components.h"
#include "Zahra/Scripting/ScriptEngine.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

// TODO: don't include this here, instead encapsulate it in a physics library
#include <box2d/b2_body.h>

namespace Zahra
{
	static std::unordered_map <MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFns;

	namespace InternalCalls
	{
		#pragma region ENGINE CORE

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// INPUT
		static bool Input_IsKeyDown(KeyCode key)
		{
			return Input::IsKeyPressed(key);
		}

		static bool Input_IsMouseButtonPressed(MouseCode button)
		{
			return Input::IsMouseButtonPressed(button);
		}

		static void Input_GetMousePos(float* outX, float* outY)
		{
			auto [x, y] = Input::GetMousePos();
			*outX = x;
			*outY = y;
		}

		static float Input_GetMouseX()
		{
			return Input::GetMouseX();
		}

		static float Input_GetMouseY()
		{
			return Input::GetMouseY();
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// LOGGING
		static void Log_Trace(MonoString* log)
		{
			char* logChars = mono_string_to_utf8(log);
			Z_SCRIPT_TRACE(logChars);
			mono_free(logChars); // NOTE: manually free things mono has allocated for us!
		}

		static void Log_Info(MonoString* log)
		{
			char* logChars = mono_string_to_utf8(log);
			Z_SCRIPT_INFO(logChars);
			mono_free(logChars);
		}

		static void Log_Warn(MonoString* log)
		{
			char* logChars = mono_string_to_utf8(log);
			Z_SCRIPT_WARN(logChars);
			mono_free(logChars);
		}

		static void Log_Error(MonoString* log)
		{
			char* logChars = mono_string_to_utf8(log);
			Z_SCRIPT_ERROR(logChars);
			mono_free(logChars);
		}

		static void Log_Critical(MonoString* log)
		{
			char* logChars = mono_string_to_utf8(log);
			Z_SCRIPT_CRITICAL(logChars);
			mono_free(logChars);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// WINDOW
		static float Window_GetWidth()
		{
			return (float)Application::Get().GetWindow().GetWidth();
		}

		static float Window_GetHeight()
		{
			return (float)Application::Get().GetWindow().GetHeight();
		}

		#pragma endregion

		#pragma region ECS
		///////////////////////////////////////////////////////////////////////////////////////////////////
		// ENTITY
		static bool Entity_HasComponent(UUID uuid, MonoReflectionType* componentType)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			Z_CORE_ASSERT(entity, "No Entity with this ID exists in the current Scene")

			MonoType* managedType = mono_reflection_type_get_type(componentType);
			Z_CORE_ASSERT(s_EntityHasComponentFns.find(managedType) != s_EntityHasComponentFns.end(),
				"This reflected component type was never registered with ScriptGlue");
			
			return s_EntityHasComponentFns.at(managedType)(entity);
		}

		static uint64_t Entity_FindEntityByName(MonoString* name)
		{
			auto entity = ScriptEngine::GetEntity(name);

			if ((bool)entity)
				return entity.GetComponents<IDComponent>().ID;

			return 0;
		}

		static MonoString* Entity_GetName(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return ScriptEngine::StdStringToMonoString(entity.GetComponents<TagComponent>().Tag);
		}

		static MonoObject* Entity_GetScriptInstance(UUID uuid)
		{
			return ScriptEngine::GetMonoObject(uuid);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// TRANSFORM COMPONENT
		static void TransformComponent_GetTranslation(UUID uuid, glm::vec3* translation)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			*translation = entity.GetComponents<TransformComponent>().Translation;
		}

		static void TransformComponent_SetTranslation(UUID uuid, glm::vec3* translation)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<TransformComponent>().Translation = *translation;
		}

		static void TransformComponent_GetEulers(UUID uuid, glm::vec3* eulers)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			*eulers = entity.GetComponents<TransformComponent>().GetEulers();
		}

		static void TransformComponent_SetEulers(UUID uuid, glm::vec3* eulers)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<TransformComponent>().SetRotation(*eulers);
		}

		static void TransformComponent_GetScale(UUID uuid, glm::vec3* scale)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			*scale = entity.GetComponents<TransformComponent>().Scale;
		}

		static void TransformComponent_SetScale(UUID uuid, glm::vec3* scale)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<TransformComponent>().Translation = *scale;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SPRITE COMPONENT
		static void SpriteComponent_GetTint(UUID uuid, glm::vec4* tint)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			*tint = entity.GetComponents<SpriteComponent>().Tint;
		}

		static void SpriteComponent_SetTint(UUID uuid, glm::vec4* tint)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<SpriteComponent>().Tint = *tint;
		}

		// TODO: texture and animation data


		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COMPONENT
		static void CircleComponent_GetColour(UUID uuid, glm::vec4* colour)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			*colour = entity.GetComponents<CircleComponent>().Colour;
		}

		static void CircleComponent_SetColour(UUID uuid, glm::vec4* colour)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CircleComponent>().Colour = *colour;
		}

		static float CircleComponent_GetThickness(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<CircleComponent>().Thickness;
		}

		static void CircleComponent_SetThickness(UUID uuid, float thickness)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CircleComponent>().Thickness = thickness;
		}

		static float CircleComponent_GetFade(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<CircleComponent>().Fade;
		}

		static void CircleComponent_SetFade(UUID uuid, float fade)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CircleComponent>().Fade = fade;

		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA COMPONENT
		static int CameraComponent_GetProjectionType(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return (int)entity.GetComponents<CameraComponent>().Camera.GetProjectionType();
		}

		static void CameraComponent_SetProjectionType(UUID uuid, int type)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CameraComponent>().Camera.SetProjectionType((SceneCamera::ProjectionType)type);
		}

		static float CameraComponent_GetVerticalFOV(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: return camera.GetOrthographicSize();
			case SceneCamera::ProjectionType::Perspective: return camera.GetPerspectiveFOV();
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
			return 0;
		}

		static void CameraComponent_SetVerticalFOV(UUID uuid, float size)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: camera.SetOrthographicSize(size); return;
			case SceneCamera::ProjectionType::Perspective: camera.SetPerspectiveFOV(size); return;
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
		}

		static float CameraComponent_GetNearPlane(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: return camera.GetOrthographicNearClip();
			case SceneCamera::ProjectionType::Perspective: return camera.GetPerspectiveNearClip();
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
			return 0;
		}

		static void CameraComponent_SetNearPlane(UUID uuid, float nearPlane)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: camera.SetOrthographicNearClip(nearPlane); return;
			case SceneCamera::ProjectionType::Perspective: camera.SetPerspectiveNearClip(nearPlane); return;
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
		}

		static float CameraComponent_GetFarPlane(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: return camera.GetOrthographicFarClip();
			case SceneCamera::ProjectionType::Perspective: return camera.GetPerspectiveFarClip();
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
			return 0;
		}

		static void CameraComponent_SetFarPlane(UUID uuid, float farPlane)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: camera.SetOrthographicNearClip(farPlane); return;
			case SceneCamera::ProjectionType::Perspective: camera.SetPerspectiveNearClip(farPlane); return;
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
		}

		static bool CameraComponent_GetFixedAspectRatio(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<CameraComponent>().FixedAspectRatio;
		}

		static void CameraComponent_SetFixedAspectRatio(UUID uuid, bool fixed)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CameraComponent>().FixedAspectRatio = fixed;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SCRIPT COMPONENT
		// TODO: implement these properly once we've got scriptIDs (also decide if we
		// use Z_CORE_ASSERT(entity.HasComponents<ScriptComponent>()), or just return
		// a reserved UUID value (0?) if the component doesn't exist)
		/*static UUID ScriptComponent_GetScriptID(UUID entityID)
		{
			auto entity = ScriptEngine::GetEntity(entityID);
			auto scriptComponent = entity.GetComponents<ScriptComponent>();
			return scriptComponent.ScriptID;
		}*/

		/*static void ScriptComponent_SetScriptID(UUID entityID, UUID scriptID)
		{
			auto entity = ScriptEngine::GetEntity(entityID);
			auto scriptComponent = entity.GetComponents<ScriptComponent>();
			scriptComponent.ScriptID = scriptID;
		}*/

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// 2D RIGID BODY COMPONENT
		static void RigidBody2DComponent_ApplyLinearImpulse(UUID uuid, glm::vec2* impulse, bool wake)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);

			// TODO: create a physics engine that can encapsulate b2 calls e.g.
			auto body = (b2Body*)entity.GetComponents<RigidBody2DComponent>().RuntimeBody;
			
			body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
		}

		static void RigidBody2DComponent_ApplyForce(UUID uuid, glm::vec2* force, bool wake)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);

			// TODO: create a physics engine that can encapsulate b2 calls e.g.
			auto body = (b2Body*)entity.GetComponents<RigidBody2DComponent>().RuntimeBody;

			body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
		}

		static void RigidBody2DComponent_ApplyTorque(UUID uuid, float torque, bool wake)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);

			// TODO: create a physics engine that can encapsulate b2 calls e.g.
			auto body = (b2Body*)entity.GetComponents<RigidBody2DComponent>().RuntimeBody;

			body->ApplyTorque(torque, wake);
		}

		static void RigidBody2DComponent_GetVelocity(UUID uuid, glm::vec2* velocity)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);

			// TODO: create a physics engine that can encapsulate b2 calls e.g.
			auto body = (b2Body*)entity.GetComponents<RigidBody2DComponent>().RuntimeBody;

			*velocity = { body->GetLinearVelocity().x, body->GetLinearVelocity().y };
		}

		static float RigidBody2DComponent_GetAngularVelocity(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);

			// TODO: create a physics engine that can encapsulate b2 calls e.g.
			auto body = (b2Body*)entity.GetComponents<RigidBody2DComponent>().RuntimeBody;

			return body->GetAngularVelocity();
		}

		static int RigidBody2DComponent_GetBodyType(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return (int)entity.GetComponents<RigidBody2DComponent>().Type;
		}

		static void RigidBody2DComponent_SetBodyType(UUID uuid, int type)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<RigidBody2DComponent>().Type = (RigidBody2DComponent::BodyType)type;
		}

		static bool RigidBody2DComponent_GetFixedRotation(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<RigidBody2DComponent>().FixedRotation;
		}

		static void RigidBody2DComponent_SetFixedRotation(UUID uuid, bool fixed)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<RigidBody2DComponent>().FixedRotation = fixed;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// RECTANGULAR COLLIDER COMPONENT
		static void RectColliderComponent_GetOffset(UUID uuid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			*offset = entity.GetComponents<RectColliderComponent>().Offset;
		}

		static void RectColliderComponent_SetOffset(UUID uuid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<RectColliderComponent>().Offset = *offset;
		}

		static void RectColliderComponent_GetHalfExtent(UUID uuid, glm::vec2* halfExtent)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			*halfExtent = entity.GetComponents<RectColliderComponent>().HalfExtent;
		}

		static void RectColliderComponent_SetHalfExtent(UUID uuid, glm::vec2* halfExtent)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<RectColliderComponent>().HalfExtent = *halfExtent;
		}

		static float RectColliderComponent_GetDensity(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<RectColliderComponent>().Density;
		}

		static void RectColliderComponent_SetDensity(UUID uuid, float density)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<RectColliderComponent>().Density = density;
		}

		static float RectColliderComponent_GetFriction(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<RectColliderComponent>().Friction;
		}

		static void RectColliderComponent_SetFriction(UUID uuid, float friction)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<RectColliderComponent>().Friction = friction;
		}

		static float RectColliderComponent_GetRestitution(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<RectColliderComponent>().Restitution;
		}

		static void RectColliderComponent_SetRestitution(UUID uuid, float restitution)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<RectColliderComponent>().Restitution = restitution;
		}

		static float RectColliderComponent_GetRestitutionThreshold(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<RectColliderComponent>().RestitutionThreshold;
		}

		static void RectColliderComponent_SetRestitutionThreshold(UUID uuid, float threshold)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<RectColliderComponent>().RestitutionThreshold = threshold;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COLLIDER COMPONENT
		static void CircleColliderComponent_GetOffset(UUID uuid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			*offset = entity.GetComponents<CircleColliderComponent>().Offset;
		}

		static void CircleColliderComponent_SetOffset(UUID uuid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CircleColliderComponent>().Offset = *offset;
		}

		static float CircleColliderComponent_GetRadius(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<CircleColliderComponent>().Radius;
		}

		static void CircleColliderComponent_SetRadius(UUID uuid, float radius)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CircleColliderComponent>().Radius = radius;
		}

		static float CircleColliderComponent_GetDensity(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<CircleColliderComponent>().Density;
		}

		static void CircleColliderComponent_SetDensity(UUID uuid, float density)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CircleColliderComponent>().Density = density;
		}

		static float CircleColliderComponent_GetFriction(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<CircleColliderComponent>().Friction;
		}

		static void CircleColliderComponent_SetFriction(UUID uuid, float friction)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CircleColliderComponent>().Friction = friction;
		}

		static float CircleColliderComponent_GetRestitution(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<CircleColliderComponent>().Restitution;
		}

		static void CircleColliderComponent_SetRestitution(UUID uuid, float restitution)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CircleColliderComponent>().Restitution = restitution;
		}

		static float CircleColliderComponent_GetRestitutionThreshold(UUID uuid)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			return entity.GetComponents<CircleColliderComponent>().RestitutionThreshold;
		}

		static void CircleColliderComponent_SetRestitutionThreshold(UUID uuid, float threshold)
		{
			Entity entity = ScriptEngine::GetEntity(uuid);
			entity.GetComponents<CircleColliderComponent>().RestitutionThreshold = threshold;
		}

		#pragma endregion
	}	

	template <typename... ComponentTypes>
	static void RegisterType(MonoImage* assemblyImage)
	{
		([&]()
			{
				std::string managedTypename = typeid(ComponentTypes).name();

				size_t trimPoint = managedTypename.find_last_of(":");
				Z_CORE_ASSERT(trimPoint != std::string::npos, "managedTypename is not of the anticipated form 'struct Zahra::...'");

				managedTypename = "Djinn." + managedTypename.substr(trimPoint + 1);
				//Z_CORE_TRACE("Script engine has registered component type '{}'", managedTypename);

				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), assemblyImage);
				Z_CORE_ASSERT(managedType, "No such type can be found in Djinn");

				s_EntityHasComponentFns[managedType] = [](Entity entity) { return entity.HasComponents<ComponentTypes>(); };
			}
		(), ...);
	}

	template<typename... ComponentTypes>
	static void RegisterType(ComponentGroup<ComponentTypes...>, MonoImage* assemblyImage)
	{
		RegisterType<ComponentTypes...>(assemblyImage);
	}

	void ScriptGlue::RegisterComponentTypes(MonoImage* assemblyImage)
	{
		s_EntityHasComponentFns.clear();
		RegisterType(MostComponents{}, assemblyImage);
	}

#define Z_REGISTER_INTERNAL_CALL(name) mono_add_internal_call("Djinn.Zahra::"#name, (void*)InternalCalls::name);

	void ScriptGlue::RegisterFunctions()
	{
		#pragma region CORE ENGINE

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// INPUT
		Z_REGISTER_INTERNAL_CALL(Input_IsKeyDown);
		Z_REGISTER_INTERNAL_CALL(Input_IsMouseButtonPressed);
		Z_REGISTER_INTERNAL_CALL(Input_GetMousePos);
		Z_REGISTER_INTERNAL_CALL(Input_GetMouseX);
		Z_REGISTER_INTERNAL_CALL(Input_GetMouseY);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// LOGGING
		Z_REGISTER_INTERNAL_CALL(Log_Trace);
		Z_REGISTER_INTERNAL_CALL(Log_Info);
		Z_REGISTER_INTERNAL_CALL(Log_Warn);
		Z_REGISTER_INTERNAL_CALL(Log_Error);
		Z_REGISTER_INTERNAL_CALL(Log_Critical);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// WINDOW
		Z_REGISTER_INTERNAL_CALL(Window_GetWidth);
		Z_REGISTER_INTERNAL_CALL(Window_GetHeight);

		#pragma endregion
		
		#pragma region ECS

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// ENTITY
		Z_REGISTER_INTERNAL_CALL(Entity_HasComponent);
		Z_REGISTER_INTERNAL_CALL(Entity_FindEntityByName);
		Z_REGISTER_INTERNAL_CALL(Entity_GetName);
		Z_REGISTER_INTERNAL_CALL(Entity_GetScriptInstance);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// TRANSFORM COMPONENT
		Z_REGISTER_INTERNAL_CALL(TransformComponent_GetTranslation);
		Z_REGISTER_INTERNAL_CALL(TransformComponent_SetTranslation);
		Z_REGISTER_INTERNAL_CALL(TransformComponent_GetEulers);
		Z_REGISTER_INTERNAL_CALL(TransformComponent_SetEulers);
		Z_REGISTER_INTERNAL_CALL(TransformComponent_GetScale);
		Z_REGISTER_INTERNAL_CALL(TransformComponent_SetScale);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SPRITE COMPONENT
		Z_REGISTER_INTERNAL_CALL(SpriteComponent_GetTint);
		Z_REGISTER_INTERNAL_CALL(SpriteComponent_SetTint);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COMPONENT
		Z_REGISTER_INTERNAL_CALL(CircleComponent_GetColour);
		Z_REGISTER_INTERNAL_CALL(CircleComponent_SetColour);
		Z_REGISTER_INTERNAL_CALL(CircleComponent_GetThickness);
		Z_REGISTER_INTERNAL_CALL(CircleComponent_SetThickness);
		Z_REGISTER_INTERNAL_CALL(CircleComponent_GetFade);
		Z_REGISTER_INTERNAL_CALL(CircleComponent_SetFade);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA COMPONENT
		Z_REGISTER_INTERNAL_CALL(CameraComponent_GetProjectionType);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_SetProjectionType);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_GetVerticalFOV);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_SetVerticalFOV);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_GetNearPlane);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_SetNearPlane);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_GetFarPlane);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_SetFarPlane);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_GetFixedAspectRatio);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_SetFixedAspectRatio);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SCRIPT COMPONENT
		//Z_REGISTER_INTERNAL_CALL(ScriptComponent_GetScriptName);
		//Z_REGISTER_INTERNAL_CALL(ScriptComponent_SetScriptName);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// 2D RIGID BODY COMPONENT
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_ApplyForce);
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_ApplyTorque);
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetVelocity);
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetAngularVelocity);

		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetBodyType);
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_SetBodyType);
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetFixedRotation);
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_SetFixedRotation);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// RECTANGULAR COLLIDER COMPONENT
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_GetOffset);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_SetOffset);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_GetHalfExtent);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_SetHalfExtent);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_GetDensity);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_SetDensity);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_GetFriction);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_SetFriction);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_GetRestitution);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_SetRestitution);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_GetRestitutionThreshold);
		Z_REGISTER_INTERNAL_CALL(RectColliderComponent_SetRestitutionThreshold);
		

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COLLIDER COMPONENT
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_GetOffset);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_SetOffset);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_GetRadius);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_SetRadius);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_GetDensity);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_SetDensity);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_GetFriction);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_SetFriction);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_GetRestitution);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_SetRestitution);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_GetRestitutionThreshold);
		Z_REGISTER_INTERNAL_CALL(CircleColliderComponent_SetRestitutionThreshold);

		#pragma endregion
	}
}
