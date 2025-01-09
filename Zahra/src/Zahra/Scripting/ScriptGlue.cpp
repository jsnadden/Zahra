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
		static bool Entity_HasComponent(ZGUID guid, MonoReflectionType* componentType)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			Z_CORE_ASSERT(entity, "No Entity with this ID exists in the current Scene")

			MonoType* managedType = mono_reflection_type_get_type(componentType);
			Z_CORE_ASSERT(s_EntityHasComponentFns.find(managedType) != s_EntityHasComponentFns.end(),
				"This reflected component type was never registered with ScriptGlue");
			
			return s_EntityHasComponentFns.at(managedType)(entity);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// TRANSFORM COMPONENT
		static void TransformComponent_GetTranslation(ZGUID guid, glm::vec3* translation)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			*translation = entity.GetComponents<TransformComponent>().Translation;
		}

		static void TransformComponent_SetTranslation(ZGUID guid, glm::vec3* translation)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<TransformComponent>().Translation = *translation;
		}

		static void TransformComponent_GetEulers(ZGUID guid, glm::vec3* eulers)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			*eulers = entity.GetComponents<TransformComponent>().GetEulers();
		}

		static void TransformComponent_SetEulers(ZGUID guid, glm::vec3* eulers)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<TransformComponent>().SetRotation(*eulers);
		}

		static void TransformComponent_GetScale(ZGUID guid, glm::vec3* scale)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			*scale = entity.GetComponents<TransformComponent>().Scale;
		}

		static void TransformComponent_SetScale(ZGUID guid, glm::vec3* scale)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<TransformComponent>().Translation = *scale;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SPRITE COMPONENT
		static void SpriteComponent_GetTint(ZGUID guid, glm::vec4* tint)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			*tint = entity.GetComponents<SpriteComponent>().Tint;
		}

		static void SpriteComponent_SetTint(ZGUID guid, glm::vec4* tint)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<SpriteComponent>().Tint = *tint;
		}

		// TODO: texture and animation data


		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COMPONENT
		static void CircleComponent_GetColour(ZGUID guid, glm::vec4* colour)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			*colour = entity.GetComponents<CircleComponent>().Colour;
		}

		static void CircleComponent_SetColour(ZGUID guid, glm::vec4* colour)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CircleComponent>().Colour = *colour;
		}

		static float CircleComponent_GetThickness(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<CircleComponent>().Thickness;
		}

		static void CircleComponent_SetThickness(ZGUID guid, float thickness)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CircleComponent>().Thickness = thickness;
		}

		static float CircleComponent_GetFade(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<CircleComponent>().Fade;
		}

		static void CircleComponent_SetFade(ZGUID guid, float fade)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CircleComponent>().Fade = fade;

		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA COMPONENT
		static int CameraComponent_GetProjectionType(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return (int)entity.GetComponents<CameraComponent>().Camera.GetProjectionType();
		}

		static void CameraComponent_SetProjectionType(ZGUID guid, int type)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CameraComponent>().Camera.SetProjectionType((SceneCamera::ProjectionType)type);
		}

		static float CameraComponent_GetVerticalFOV(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: return camera.GetOrthographicSize();
			case SceneCamera::ProjectionType::Perspective: return camera.GetPerspectiveFOV();
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
			return 0;
		}

		static void CameraComponent_SetVerticalFOV(ZGUID guid, float size)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: camera.SetOrthographicSize(size); return;
			case SceneCamera::ProjectionType::Perspective: camera.SetPerspectiveFOV(size); return;
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
		}

		static float CameraComponent_GetNearPlane(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: return camera.GetOrthographicNearClip();
			case SceneCamera::ProjectionType::Perspective: return camera.GetPerspectiveNearClip();
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
			return 0;
		}

		static void CameraComponent_SetNearPlane(ZGUID guid, float nearPlane)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: camera.SetOrthographicNearClip(nearPlane); return;
			case SceneCamera::ProjectionType::Perspective: camera.SetPerspectiveNearClip(nearPlane); return;
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
		}

		static float CameraComponent_GetFarPlane(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: return camera.GetOrthographicFarClip();
			case SceneCamera::ProjectionType::Perspective: return camera.GetPerspectiveFarClip();
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
			return 0;
		}

		static void CameraComponent_SetFarPlane(ZGUID guid, float farPlane)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: camera.SetOrthographicNearClip(farPlane); return;
			case SceneCamera::ProjectionType::Perspective: camera.SetPerspectiveNearClip(farPlane); return;
			}
			Z_CORE_ASSERT(false, "Invalid ProjectionType value");
		}

		static bool CameraComponent_GetFixedAspectRatio(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<CameraComponent>().FixedAspectRatio;
		}

		static void CameraComponent_SetFixedAspectRatio(ZGUID guid, bool fixed)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CameraComponent>().FixedAspectRatio = fixed;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SCRIPT COMPONENT
		static MonoString* ScriptComponent_GetScriptName(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return ScriptEngine::StdStringToMonoString(entity.GetComponents<ScriptComponent>().ScriptName);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// 2D RIGID BODY COMPONENT
		static void RigidBody2DComponent_ApplyLinearImpulse(ZGUID guid, glm::vec2* impulse, bool wake)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);

			// TODO: create a physics engine that can encapsulate b2 calls e.g.
			auto body = (b2Body*)entity.GetComponents<RigidBody2DComponent>().RuntimeBody;
			
			body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
		}

		static void RigidBody2DComponent_ApplyForce(ZGUID guid, glm::vec2* force, bool wake)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);

			// TODO: create a physics engine that can encapsulate b2 calls e.g.
			auto body = (b2Body*)entity.GetComponents<RigidBody2DComponent>().RuntimeBody;

			body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
		}

		static void RigidBody2DComponent_GetVelocity(ZGUID guid, glm::vec2* velocity)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);

			// TODO: create a physics engine that can encapsulate b2 calls e.g.
			auto body = (b2Body*)entity.GetComponents<RigidBody2DComponent>().RuntimeBody;

			*velocity = { body->GetLinearVelocity().x, body->GetLinearVelocity().y };
		}

		static int RigidBody2DComponent_GetBodyType(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return (int)entity.GetComponents<RigidBody2DComponent>().Type;
		}

		static void RigidBody2DComponent_SetBodyType(ZGUID guid, int type)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<RigidBody2DComponent>().Type = (RigidBody2DComponent::BodyType)type;
		}

		static bool RigidBody2DComponent_GetFixedRotation(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<RigidBody2DComponent>().FixedRotation;
		}

		static void RigidBody2DComponent_SetFixedRotation(ZGUID guid, bool fixed)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<RigidBody2DComponent>().FixedRotation = fixed;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// RECTANGULAR COLLIDER COMPONENT
		static void RectColliderComponent_GetOffset(ZGUID guid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			*offset = entity.GetComponents<RectColliderComponent>().Offset;
		}

		static void RectColliderComponent_SetOffset(ZGUID guid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<RectColliderComponent>().Offset = *offset;
		}

		static void RectColliderComponent_GetHalfExtent(ZGUID guid, glm::vec2* halfExtent)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			*halfExtent = entity.GetComponents<RectColliderComponent>().HalfExtent;
		}

		static void RectColliderComponent_SetHalfExtent(ZGUID guid, glm::vec2* halfExtent)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<RectColliderComponent>().HalfExtent = *halfExtent;
		}

		static float RectColliderComponent_GetDensity(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<RectColliderComponent>().Density;
		}

		static void RectColliderComponent_SetDensity(ZGUID guid, float density)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<RectColliderComponent>().Density = density;
		}

		static float RectColliderComponent_GetFriction(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<RectColliderComponent>().Friction;
		}

		static void RectColliderComponent_SetFriction(ZGUID guid, float friction)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<RectColliderComponent>().Friction = friction;
		}

		static float RectColliderComponent_GetRestitution(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<RectColliderComponent>().Restitution;
		}

		static void RectColliderComponent_SetRestitution(ZGUID guid, float restitution)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<RectColliderComponent>().Restitution = restitution;
		}

		static float RectColliderComponent_GetRestitutionThreshold(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<RectColliderComponent>().RestitutionThreshold;
		}

		static void RectColliderComponent_SetRestitutionThreshold(ZGUID guid, float threshold)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<RectColliderComponent>().RestitutionThreshold = threshold;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COLLIDER COMPONENT
		static void CircleColliderComponent_GetOffset(ZGUID guid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			*offset = entity.GetComponents<CircleColliderComponent>().Offset;
		}

		static void CircleColliderComponent_SetOffset(ZGUID guid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CircleColliderComponent>().Offset = *offset;
		}

		static float CircleColliderComponent_GetRadius(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<CircleColliderComponent>().Radius;
		}

		static void CircleColliderComponent_SetRadius(ZGUID guid, float radius)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CircleColliderComponent>().Radius = radius;
		}

		static float CircleColliderComponent_GetDensity(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<CircleColliderComponent>().Density;
		}

		static void CircleColliderComponent_SetDensity(ZGUID guid, float density)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CircleColliderComponent>().Density = density;
		}

		static float CircleColliderComponent_GetFriction(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<CircleColliderComponent>().Friction;
		}

		static void CircleColliderComponent_SetFriction(ZGUID guid, float friction)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CircleColliderComponent>().Friction = friction;
		}

		static float CircleColliderComponent_GetRestitution(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<CircleColliderComponent>().Restitution;
		}

		static void CircleColliderComponent_SetRestitution(ZGUID guid, float restitution)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			entity.GetComponents<CircleColliderComponent>().Restitution = restitution;
		}

		static float CircleColliderComponent_GetRestitutionThreshold(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
			return entity.GetComponents<CircleColliderComponent>().RestitutionThreshold;
		}

		static void CircleColliderComponent_SetRestitutionThreshold(ZGUID guid, float threshold)
		{
			Entity entity = ScriptEngine::GetEntityFromGUID(guid);
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
		Z_REGISTER_INTERNAL_CALL(ScriptComponent_GetScriptName);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// 2D RIGID BODY COMPONENT
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_ApplyForce);
		Z_REGISTER_INTERNAL_CALL(RigidBody2DComponent_GetVelocity);
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
