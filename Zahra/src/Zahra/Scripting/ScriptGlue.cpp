#include "zpch.h"
#include "ScriptGlue.h"

#include "Zahra/Scripting/ScriptEngine.h"
#include "Zahra/Core/Application.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

namespace Zahra
{
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
			return Application::Get().GetWindow().GetWidth();
		}

		static float Window_GetHeight()
		{
			return Application::Get().GetWindow().GetHeight();
		}

#pragma endregion

#pragma region ECS

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// ENTITY
		/*template <typename T>
		static bool Entity_HasComponent(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.HasComponents<T>();
		}*/

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// TRANSFORM COMPONENT
		static void TransformComponent_GetTranslation(ZGUID guid, glm::vec3* translation)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			*translation = entity.GetComponents<TransformComponent>().Translation;
		}

		static void TransformComponent_SetTranslation(ZGUID guid, glm::vec3* translation)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<TransformComponent>().Translation = *translation;
		}

		static void TransformComponent_GetEulers(ZGUID guid, glm::vec3* eulers)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			*eulers = entity.GetComponents<TransformComponent>().EulerAngles;
		}

		static void TransformComponent_SetEulers(ZGUID guid, glm::vec3* eulers)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<TransformComponent>().EulerAngles = *eulers;
		}

		static void TransformComponent_GetScale(ZGUID guid, glm::vec3* scale)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			*scale = entity.GetComponents<TransformComponent>().Scale;
		}

		static void TransformComponent_SetScale(ZGUID guid, glm::vec3* scale)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<TransformComponent>().Translation = *scale;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SPRITE COMPONENT
		static void SpriteComponent_GetTint(ZGUID guid, glm::vec4* tint)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			*tint = entity.GetComponents<SpriteComponent>().Tint;
		}

		static void SpriteComponent_SetTint(ZGUID guid, glm::vec4* tint)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<SpriteComponent>().Tint = *tint;
		}

		// TODO: texture and animation data


		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COMPONENT
		static void CircleComponent_GetColour(ZGUID guid, glm::vec4* colour)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			*colour = entity.GetComponents<CircleComponent>().Colour;
		}

		static void CircleComponent_SetColour(ZGUID guid, glm::vec4* colour)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CircleComponent>().Colour = *colour;
		}

		static float CircleComponent_GetThickness(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<CircleComponent>().Thickness;
		}

		static void CircleComponent_SetThickness(ZGUID guid, float thickness)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CircleComponent>().Thickness = thickness;
		}

		static float CircleComponent_GetFade(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<CircleComponent>().Fade;
		}

		static void CircleComponent_SetFade(ZGUID guid, float fade)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CircleComponent>().Fade = fade;

		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA COMPONENT
		static int CameraComponent_GetProjectionType(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return (int)entity.GetComponents<CameraComponent>().Camera.GetProjectionType();
		}

		static void CameraComponent_SetProjectionType(ZGUID guid, int type)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CameraComponent>().Camera.SetProjectionType((SceneCamera::ProjectionType)type);
		}

		static float CameraComponent_GetVerticalSize(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: return camera.GetOrthographicSize();
			case SceneCamera::ProjectionType::Perspective: return camera.GetPerspectiveFOV();
			}
		}

		static void CameraComponent_SetVerticalSize(ZGUID guid, float size)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: camera.SetOrthographicSize(size);
			case SceneCamera::ProjectionType::Perspective: camera.SetPerspectiveFOV(size);
			}
		}

		static float CameraComponent_GetNearPlane(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: return camera.GetOrthographicNearClip();
			case SceneCamera::ProjectionType::Perspective: return camera.GetPerspectiveNearClip();
			}
		}

		static void CameraComponent_SetNearPlane(ZGUID guid, float nearPlane)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: camera.SetOrthographicNearClip(nearPlane);
			case SceneCamera::ProjectionType::Perspective: camera.SetPerspectiveNearClip(nearPlane);
			}
		}

		static float CameraComponent_GetFarPlane(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: return camera.GetOrthographicFarClip();
			case SceneCamera::ProjectionType::Perspective: return camera.GetPerspectiveFarClip();
			}
		}

		static void CameraComponent_SetFarPlane(ZGUID guid, float farPlane)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			SceneCamera& camera = entity.GetComponents<CameraComponent>().Camera;

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Orthographic: camera.SetOrthographicNearClip(farPlane);
			case SceneCamera::ProjectionType::Perspective: camera.SetPerspectiveNearClip(farPlane);
			}
		}

		static bool CameraComponent_GetFixedAspectRatio(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<CameraComponent>().FixedAspectRatio;
		}

		static void CameraComponent_SetFixedAspectRatio(ZGUID guid, bool fixed)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CameraComponent>().FixedAspectRatio = fixed;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SCRIPT COMPONENT
		static MonoString* ScriptComponent_GetScriptName(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return (MonoString*)ScriptEngine::GetMonoString(entity.GetComponents<ScriptComponent>().ScriptName);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// 2D RIGID BODY COMPONENT
		static int RigidBody2DComponent_GetBodyType(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return (int)entity.GetComponents<RigidBody2DComponent>().Type;
		}

		static void RigidBody2DComponent_SetBodyType(ZGUID guid, int type)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<RigidBody2DComponent>().Type = (RigidBody2DComponent::BodyType)type;
		}

		static bool RigidBody2DComponent_GetFixedRotation(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<RigidBody2DComponent>().FixedRotation;
		}

		static void RigidBody2DComponent_SetFixedRotation(ZGUID guid, bool fixed)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<RigidBody2DComponent>().FixedRotation = fixed;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// RECTANGULAR COLLIDER COMPONENT
		static void RectColliderComponent_GetOffset(ZGUID guid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			*offset = entity.GetComponents<RectColliderComponent>().Offset;
		}

		static void RectColliderComponent_SetOffset(ZGUID guid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<RectColliderComponent>().Offset = *offset;
		}

		static void RectColliderComponent_GetHalfExtent(ZGUID guid, glm::vec2* halfExtent)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			*halfExtent = entity.GetComponents<RectColliderComponent>().HalfExtent;
		}

		static void RectColliderComponent_SetHalfExtent(ZGUID guid, glm::vec2* halfExtent)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<RectColliderComponent>().HalfExtent = *halfExtent;
		}

		static float RectColliderComponent_GetDensity(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<RectColliderComponent>().Density;
		}

		static void RectColliderComponent_SetDensity(ZGUID guid, float density)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<RectColliderComponent>().Density = density;
		}

		static float RectColliderComponent_GetFriction(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<RectColliderComponent>().Friction;
		}

		static void RectColliderComponent_SetFriction(ZGUID guid, float friction)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<RectColliderComponent>().Friction = friction;
		}

		static float RectColliderComponent_GetRestitution(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<RectColliderComponent>().Restitution;
		}

		static void RectColliderComponent_SetRestitution(ZGUID guid, float restitution)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<RectColliderComponent>().Restitution = restitution;
		}

		static float RectColliderComponent_GetRestitutionThreshold(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<RectColliderComponent>().RestitutionThreshold;
		}

		static void RectColliderComponent_SetRestitutionThreshold(ZGUID guid, float threshold)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<RectColliderComponent>().RestitutionThreshold = threshold;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COLLIDER COMPONENT
		static void CircleColliderComponent_GetOffset(ZGUID guid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			*offset = entity.GetComponents<CircleColliderComponent>().Offset;
		}

		static void CircleColliderComponent_SetOffset(ZGUID guid, glm::vec2* offset)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CircleColliderComponent>().Offset = *offset;
		}

		static float CircleColliderComponent_GetRadius(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<CircleColliderComponent>().Radius;
		}

		static void CircleColliderComponent_SetRadius(ZGUID guid, float radius)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CircleColliderComponent>().Radius = radius;
		}

		static float CircleColliderComponent_GetDensity(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<CircleColliderComponent>().Density;
		}

		static void CircleColliderComponent_SetDensity(ZGUID guid, float density)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CircleColliderComponent>().Density = density;
		}

		static float CircleColliderComponent_GetFriction(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<CircleColliderComponent>().Friction;
		}

		static void CircleColliderComponent_SetFriction(ZGUID guid, float friction)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CircleColliderComponent>().Friction = friction;
		}

		static float CircleColliderComponent_GetRestitution(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<CircleColliderComponent>().Restitution;
		}

		static void CircleColliderComponent_SetRestitution(ZGUID guid, float restitution)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CircleColliderComponent>().Restitution = restitution;
		}

		static float CircleColliderComponent_GetRestitutionThreshold(ZGUID guid)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			return entity.GetComponents<CircleColliderComponent>().RestitutionThreshold;
		}

		static void CircleColliderComponent_SetRestitutionThreshold(ZGUID guid, float threshold)
		{
			Entity entity = ScriptEngine::GetEntity(guid);
			entity.GetComponents<CircleColliderComponent>().RestitutionThreshold = threshold;
		}

		#pragma endregion

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
		Z_REGISTER_INTERNAL_CALL(CameraComponent_GetVerticalSize);
		Z_REGISTER_INTERNAL_CALL(CameraComponent_SetVerticalSize);
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

