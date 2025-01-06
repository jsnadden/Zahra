

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

		// TODO: not much point in a script setting these parameters, unless we can also
		// trigger a reset of the box2d physicsworld...
		/*BodyType
		FixedRotation*/
	}

	public class RectColliderComponent : Component
	{
		// TODO: not much point in a script setting these parameters, unless we can also
		// trigger a reset of the box2d physicsworld...
		/*Offset;
		HalfExtent;
		Density;
		Friction;
		Restitution;
		RestitutionThreshold;*/
	}

	public class CircleColliderComponent : Component
	{
		// TODO: not much point in a script setting these parameters, unless we can also
		// trigger a reset of the box2d physicsworld...
		/*Offset;
		Radius;
		Density;
		Friction;
		Restitution;
		RestitutionThreshold;*/
	}

}
