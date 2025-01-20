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
		internal extern static bool Input_IsMouseButtonPressed(MouseCode button);

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
		internal extern static bool Entity_HasComponent(ulong uuid, Type componentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Entity_GetName(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static object Entity_GetScriptInstance(ulong uuid);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// TRANSFORM COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong uuid, out Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong uuid, ref Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetEulers(ulong uuid, out Vector3 eulers);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetEulers(ulong uuid, ref Vector3 eulers);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong uuid, out Vector3 scale);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong uuid, ref Vector3 scale);		

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SPRITE COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SpriteComponent_GetTint(ulong uuid, out Vector4 tint);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void SpriteComponent_SetTint(ulong uuid, ref Vector4 tint);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleComponent_GetColour(ulong uuid, out Vector4 tint);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleComponent_SetColour(ulong uuid, ref Vector4 tint);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleComponent_GetThickness(ulong uuid);
			
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleComponent_SetThickness(ulong uuid, float thickness);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleComponent_GetFade(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleComponent_SetFade(ulong uuid, float fade);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ProjectionType CameraComponent_GetProjectionType(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetProjectionType(ulong uuid, int type);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetVerticalFOV(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetVerticalFOV(ulong uuid, float size);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetNearPlane(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetNearPlane(ulong uuid, float nearPlane);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CameraComponent_GetFarPlane(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFarPlane(ulong uuid, float farPlane);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool CameraComponent_GetFixedAspectRatio(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetFixedAspectRatio(ulong uuid, bool value);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SCRIPT COMPONENT
		/*[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string ScriptComponent_GetScriptName(ulong uuid);*/

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// 2D RIGID BODY COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyLinearImpulse(ulong uuid, ref Vector2 impulse, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_ApplyForce(ulong uuid, ref Vector2 force, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_GetVelocity(ulong uuid, out Vector2 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static BodyType RigidBody2DComponent_GetBodyType(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetBodyType(ulong uuid, BodyType bodyType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool RigidBody2DComponent_GetFixedRotation(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RigidBody2DComponent_SetFixedRotation(ulong uuid, bool fix);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// RECTANGULAR COLLIDER COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_GetOffset(ulong UUID,  out Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetOffset(ulong UUID, ref Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_GetHalfExtent(ulong UUID, out Vector2 halfExtent);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetHalfExtent(ulong UUID, ref Vector2 halfExtent);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float RectColliderComponent_GetDensity(ulong UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetDensity(ulong UUID, float density);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float RectColliderComponent_GetFriction(ulong UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetFriction(ulong UUID, float friction);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float RectColliderComponent_GetRestitution(ulong UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetRestitution(ulong UUID, float restitution);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float RectColliderComponent_GetRestitutionThreshold(ulong UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void RectColliderComponent_SetRestitutionThreshold(ulong UUID, float threshold);

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCULAR COLLIDER COMPONENT
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_GetOffset(ulong UUID, out Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetOffset(ulong UUID, ref Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetRadius(ulong UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetRadius(ulong UUID, float radius);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetDensity(ulong UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetDensity(ulong UUID, float density);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetFriction(ulong UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetFriction(ulong UUID, float friction);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetRestitution(ulong UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetRestitution(ulong UUID, float restitution);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float CircleColliderComponent_GetRestitutionThreshold(ulong UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void CircleColliderComponent_SetRestitutionThreshold(ulong UUID, float threshold);





		#endregion


	}
}
