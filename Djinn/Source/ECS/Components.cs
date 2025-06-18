

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
				Zahra.TransformComponent_GetTranslation(Entity.UUID, out Vector3 translation);
				return translation;
			}
			set
			{
				Zahra.TransformComponent_SetTranslation(Entity.UUID, ref value);
			}
		}

		public Vector3 EulerAngles
		{
			get
			{
				Zahra.TransformComponent_GetEulers(Entity.UUID, out Vector3 eulers);
				return eulers;
			}
			set
			{
				Zahra.TransformComponent_SetEulers(Entity.UUID, ref value);
			}
		}

		public Vector3 Scale
		{
			get
			{
				Zahra.TransformComponent_GetScale(Entity.UUID, out Vector3 scale);
				return scale;
			}
			set
			{
				Zahra.TransformComponent_SetScale(Entity.UUID, ref value);
			}
		}
	}

	public class SpriteComponent : Component
	{
		public Vector4 Tint
		{
			get
			{
				Zahra.SpriteComponent_GetTint(Entity.UUID, out Vector4 tint);
				return tint;
			}
			set
			{
				Zahra.SpriteComponent_SetTint(Entity.UUID, ref value);
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
				Zahra.CircleComponent_GetColour(Entity.UUID, out Vector4 colour);
				return colour;
			}
			set
			{
				Zahra.CircleComponent_SetColour(Entity.UUID, ref value);
			}
		}

		public float Thickness
		{
			get
			{
				return Zahra.CircleComponent_GetThickness(Entity.UUID);
			}
			set
			{
				Zahra.CircleComponent_SetThickness(Entity.UUID, value);
			}
		}

		public float Fade
		{
			get
			{
				return Zahra.CircleComponent_GetFade(Entity.UUID);
			}
			set
			{
				Zahra.CircleComponent_SetFade(Entity.UUID, value);
			}
		}
	}

	public class TextComponent : Component
	{
		// TODO: hook this up
	}

	public enum ProjectionType { Orthographic = 0, Perspective = 1 };

	public class CameraComponent : Component
	{
		public ProjectionType ProjectionType
		{
			get
			{
				return Zahra.CameraComponent_GetProjectionType(Entity.UUID);
			}
			set
			{
				Zahra.CameraComponent_SetProjectionType(Entity.UUID, (int)value);
			}
		}

		public float VerticalFOV
		{
			get
			{
				return Zahra.CameraComponent_GetVerticalFOV(Entity.UUID);
			}
			set
			{
				Zahra.CameraComponent_SetVerticalFOV(Entity.UUID, value);
			}
		}

		public float NearPlane
		{
			get
			{
				return Zahra.CameraComponent_GetNearPlane(Entity.UUID);
			}
			set
			{
				Zahra.CameraComponent_SetNearPlane(Entity.UUID, value);
			}
		}

		public float FarPlane
		{
			get
			{
				return Zahra.CameraComponent_GetFarPlane(Entity.UUID);
			}
			set
			{
				Zahra.CameraComponent_SetFarPlane(Entity.UUID, value);
			}
		}

		public bool FixedAspectRatio
		{
			get
			{
				return Zahra.CameraComponent_GetFixedAspectRatio(Entity.UUID);
			}
			set
			{
				Zahra.CameraComponent_SetFixedAspectRatio(Entity.UUID, value);
			}
		}
	}

	public class ScriptComponent : Component
	{
		
	}

	public enum BodyType { Static = 0, Dynamic = 1, Kinematic = 2 };

	// TODO: before writing setters for physics parameters, I'll need to figure 
	public class RigidBody2DComponent : Component
	{
		public void ApplyLinearImpulse(Vector2 impulse, bool wake)
		{
			Zahra.RigidBody2DComponent_ApplyLinearImpulse(Entity.UUID, ref impulse, wake);
		}

		public void ApplyForce(Vector2 force, bool wake)
		{
			Zahra.RigidBody2DComponent_ApplyForce(Entity.UUID, ref force, wake);
		}

		public Vector2 GetVelocity()
		{
			Zahra.RigidBody2DComponent_GetVelocity(Entity.UUID, out Vector2 velocity);
			return velocity;
		}

		public BodyType GetBodyType
		{
			get
			{
				return Zahra.RigidBody2DComponent_GetBodyType(Entity.UUID);
			}
			set
			{

			}
		}

		public bool FixedRotation
		{
			get
			{
				return Zahra.RigidBody2DComponent_GetFixedRotation(Entity.UUID);
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
				Zahra.RectColliderComponent_GetOffset(Entity.UUID, out Vector2 offset);
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
				Zahra.RectColliderComponent_GetHalfExtent(Entity.UUID, out Vector2 halfExtent);
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
				return Zahra.RectColliderComponent_GetDensity(Entity.UUID);
			}
			set
			{

			}
		}

		public float Friction
		{
			get
			{
				return Zahra.RectColliderComponent_GetFriction(Entity.UUID);
			}
			set
			{

			}
		}

		public float Restitution
		{
			get
			{
				return Zahra.RectColliderComponent_GetRestitution(Entity.UUID);
			}
			set
			{

			}
		}

		public float RestitutionThreshold
		{
			get
			{
				return Zahra.RectColliderComponent_GetRestitutionThreshold(Entity.UUID);
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
				Zahra.CircleColliderComponent_GetOffset(Entity.UUID, out Vector2 offset);
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
				return Zahra.CircleColliderComponent_GetRadius(Entity.UUID);
			}
			set
			{
				
			}
		}

		public float Density
		{
			get
			{
				return Zahra.CircleColliderComponent_GetDensity(Entity.UUID);
			}
			set
			{

			}
		}

		public float Friction
		{
			get
			{
				return Zahra.CircleColliderComponent_GetFriction(Entity.UUID);
			}
			set
			{

			}
		}

		public float Restitution
		{
			get
			{
				return Zahra.CircleColliderComponent_GetRestitution(Entity.UUID);
			}
			set
			{

			}
		}

		public float RestitutionThreshold
		{
			get
			{
				return Zahra.CircleColliderComponent_GetRestitutionThreshold(Entity.UUID);
			}
			set
			{

			}
		}
	}

}
