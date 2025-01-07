

using System;

namespace Djinn
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class TransformComponent : Component
	{
		public Vector3 Translation
		{
			get
			{
				Zahra.TransformComponent_GetTranslation(Entity.GUID, out Vector3 translation);
				return translation;
			}
			set
			{
				Zahra.TransformComponent_SetTranslation(Entity.GUID, ref value);
			}
		}

		public Vector3 EulerAngles
		{
			get
			{
				Zahra.TransformComponent_GetEulers(Entity.GUID, out Vector3 eulers);
				return eulers;
			}
			set
			{
				Zahra.TransformComponent_SetEulers(Entity.GUID, ref value);
			}
		}

		public Vector3 Scale
		{
			get
			{
				Zahra.TransformComponent_GetScale(Entity.GUID, out Vector3 scale);
				return scale;
			}
			set
			{
				Zahra.TransformComponent_SetScale(Entity.GUID, ref value);
			}
		}
	}

	public class SpriteComponent : Component
	{
		public Vector4 Tint
		{
			get
			{
				Zahra.SpriteComponent_GetTint(Entity.GUID, out Vector4 tint);
				return tint;
			}
			set
			{
				Zahra.SpriteComponent_SetTint(Entity.GUID, ref value);
			}
		}

		// TODO: texturing, animation, etc.
	}

	public class CircleComponent : Component
	{
		public Vector4 Colour
		{
			get
			{
				Zahra.CircleComponent_GetColour(Entity.GUID, out Vector4 colour);
				return colour;
			}
			set
			{
				Zahra.CircleComponent_SetColour(Entity.GUID, ref value);
			}
		}

		public float Thickness
		{
			get
			{
				return Zahra.CircleComponent_GetThickness(Entity.GUID);
			}
			set
			{
				Zahra.CircleComponent_SetThickness(Entity.GUID, value);
			}
		}

		public float Fade
		{
			get
			{
				return Zahra.CircleComponent_GetFade(Entity.GUID);
			}
			set
			{
				Zahra.CircleComponent_SetFade(Entity.GUID, value);
			}
		}
	}

	public enum ProjectionType { Orthographic = 0, Perspective = 1 };

	public class CameraComponent : Component
	{
		public ProjectionType ProjectionType
		{
			get
			{
				return Zahra.CameraComponent_GetProjectionType(Entity.GUID);
			}
			set
			{
				Zahra.CameraComponent_SetProjectionType(Entity.GUID, (int)value);
			}
		}

		public float VerticalFOV
		{
			get
			{
				return Zahra.CameraComponent_GetVerticalFOV(Entity.GUID);
			}
			set
			{
				Zahra.CameraComponent_SetVerticalFOV(Entity.GUID, value);
			}
		}

		public float NearPlane
		{
			get
			{
				return Zahra.CameraComponent_GetNearPlane(Entity.GUID);
			}
			set
			{
				Zahra.CameraComponent_SetNearPlane(Entity.GUID, value);
			}
		}

		public float FarPlane
		{
			get
			{
				return Zahra.CameraComponent_GetFarPlane(Entity.GUID);
			}
			set
			{
				Zahra.CameraComponent_SetFarPlane(Entity.GUID, value);
			}
		}

		public bool FixedAspectRatio
		{
			get
			{
				return Zahra.CameraComponent_GetFixedAspectRatio(Entity.GUID);
			}
			set
			{
				Zahra.CameraComponent_SetFixedAspectRatio(Entity.GUID, value);
			}
		}
	}

	public class ScriptComponent : Component
	{
		public string GetScriptName()
		{
			return Djinn.Zahra.ScriptComponent_GetScriptName(Entity.GUID);
		}
	}

	public enum BodyType { Static = 0, Dynamic = 1, Kinematic = 2 };

	// TODO: before writing setters for physics parameters, I'll need to figure 
	public class RigidBody2DComponent : Component
	{
		public void ApplyLinearImpulse(Vector2 impulse, bool wake)
		{
			Zahra.RigidBody2DComponent_ApplyLinearImpulse(Entity.GUID, ref impulse, wake);
		}

		public void ApplyForce(Vector2 force, bool wake)
		{
			Zahra.RigidBody2DComponent_ApplyForce(Entity.GUID, ref force, wake);
		}

		public Vector2 GetVelocity()
		{
			Zahra.RigidBody2DComponent_GetVelocity(Entity.GUID, out Vector2 velocity);
			return velocity;
		}

		public BodyType GetBodyType
		{
			get
			{
				return Zahra.RigidBody2DComponent_GetBodyType(Entity.GUID);
			}
			set
			{

			}
		}

		public bool FixedRotation
		{
			get
			{
				return Zahra.RigidBody2DComponent_GetFixedRotation(Entity.GUID);
			}
			set
			{

			}
		}
		/*BodyType
		FixedRotation*/
	}

	public class RectColliderComponent : Component
	{
		public Vector2 Offset
		{
			get
			{
				Zahra.RectColliderComponent_GetOffset(Entity.GUID, out Vector2 offset);
				return offset;
			}
			set
			{

			}
		}

		public Vector2 HalfExtent
		{
			get
			{
				Zahra.RectColliderComponent_GetHalfExtent(Entity.GUID, out Vector2 halfExtent);
				return halfExtent;
			}
			set
			{

			}
		}

		public float Density
		{
			get
			{
				return Zahra.RectColliderComponent_GetDensity(Entity.GUID);
			}
			set
			{

			}
		}

		public float Friction
		{
			get
			{
				return Zahra.RectColliderComponent_GetFriction(Entity.GUID);
			}
			set
			{

			}
		}

		public float Restitution
		{
			get
			{
				return Zahra.RectColliderComponent_GetRestitution(Entity.GUID);
			}
			set
			{

			}
		}

		public float RestitutionThreshold
		{
			get
			{
				return Zahra.RectColliderComponent_GetRestitutionThreshold(Entity.GUID);
			}
			set
			{

			}
		}
	}

	public class CircleColliderComponent : Component
	{
		public Vector2 Offset
		{
			get
			{
				Zahra.CircleColliderComponent_GetOffset(Entity.GUID, out Vector2 offset);
				return offset;
			}
			set
			{

			}
		}

		public float Radius
		{
			get
			{
				return Zahra.CircleColliderComponent_GetRadius(Entity.GUID);
			}
			set
			{
				
			}
		}

		public float Density
		{
			get
			{
				return Zahra.CircleColliderComponent_GetDensity(Entity.GUID);
			}
			set
			{

			}
		}

		public float Friction
		{
			get
			{
				return Zahra.CircleColliderComponent_GetFriction(Entity.GUID);
			}
			set
			{

			}
		}

		public float Restitution
		{
			get
			{
				return Zahra.CircleColliderComponent_GetRestitution(Entity.GUID);
			}
			set
			{

			}
		}

		public float RestitutionThreshold
		{
			get
			{
				return Zahra.CircleColliderComponent_GetRestitutionThreshold(Entity.GUID);
			}
			set
			{

			}
		}
	}

}
