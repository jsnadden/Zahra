using System;
using System.Runtime.CompilerServices;

namespace Djinn
{
	public static class Zahra
	{
		#region ENGINE CORE

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// INPUT
			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static bool Input_IsKeyDown(KeyCode key);

			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static bool Input_IsMouseButtonDown(MouseCode button);

			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static void Input_GetMousePos(out float x, out float y);

			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static float Input_GetMouseX();

			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static float Input_GetMouseY();

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// LOGGING
			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static void Log_Trace(string text);

			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static void Log_Info(string text);

			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static void Log_Warn(string text);

			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static void Log_Error(string text);

			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static void Log_Critical(string text);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// WINDOW
			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static float Window_GetWidth();

			[MethodImplAttribute(MethodImplOptions.InternalCall)]
			internal extern static float Window_GetHeight();

		#endregion

		#region ECS
		///////////////////////////////////////////////////////////////////////////////////////////////////
		// ENTITY
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong guid, Type componentType);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// TRANSFORM COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong guid, out Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong guid, ref Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetEulers(ulong guid, out Vector3 eulers);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetEulers(ulong guid, ref Vector3 eulers);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong guid, out Vector3 scale);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong guid, ref Vector3 scale);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SPRITE COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SpriteComponent_GetTint(ulong guid, out Vector4 tint);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SpriteComponent_SetTint(ulong guid, ref Vector4 tint);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleComponent_GetColour(ulong guid, out Vector4 tint);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleComponent_SetColour(ulong guid, ref Vector4 tint);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleComponent_GetThickness(ulong guid);
			
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleComponent_SetThickness(ulong guid, float thickness);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleComponent_GetFade(ulong guid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleComponent_SetFade(ulong guid, float fade);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ProjectionType CameraComponent_GetProjectionType(ulong guid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetProjectionType(ulong guid, int type);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetVerticalFOV(ulong guid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetVerticalFOV(ulong guid, float size);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetNearPlane(ulong guid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetNearPlane(ulong guid, float nearPlane);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetFarPlane(ulong guid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFarPlane(ulong guid, float farPlane);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool CameraComponent_GetFixedAspectRatio(ulong guid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFixedAspectRatio(ulong guid, bool value);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SCRIPT COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string ScriptComponent_GetScriptName(ulong guid);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// 2D RIGID BODY COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyLinearImpulse(ulong guid, ref Vector2 impulse, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static BodyType RigidBody2DComponent_GetBodyType(ulong guid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetBodyType(ulong guid, BodyType bodyType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBody2DComponent_GetFixedRotation(ulong guid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetFixedRotation(ulong guid, bool fix);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// RECTANGULAR COLLIDER COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_GetOffset(ulong GUID,  out Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetOffset(ulong GUID, ref Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_GetHalfExtent(ulong GUID, out Vector2 halfExtent);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetHalfExtent(ulong GUID, ref Vector2 halfExtent);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float RectColliderComponent_GetDensity(ulong GUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetDensity(ulong GUID, float density);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float RectColliderComponent_GetFriction(ulong GUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetFriction(ulong GUID, float friction);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float RectColliderComponent_GetRestitution(ulong GUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetRestitution(ulong GUID, float restitution);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float RectColliderComponent_GetRestitutionThreshold(ulong GUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetRestitutionThreshold(ulong GUID, float threshold);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCULAR COLLIDER COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_GetOffset(ulong GUID, out Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetOffset(ulong GUID, ref Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetRadius(ulong GUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetRadius(ulong GUID, float radius);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetDensity(ulong GUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetDensity(ulong GUID, float density);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetFriction(ulong GUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetFriction(ulong GUID, float friction);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetRestitution(ulong GUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetRestitution(ulong GUID, float restitution);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetRestitutionThreshold(ulong GUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetRestitutionThreshold(ulong GUID, float threshold);





		#endregion


	}
}
