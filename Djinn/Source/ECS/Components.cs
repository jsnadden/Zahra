

using System;

namespace Djinn
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class TransformComponent : Component
	{
		public Vector3 Translation = Vector3.Zero;
		public Vector3 EulerAngles = Vector3.Zero;
		public Vector3 Scale = Vector3.One;
	}

	public class SpriteComponent : Component
	{
		public Vector4 Tint = Vector4.One;
		// TODO: textures and animation
	}

	public class CircleComponent : Component
	{
		public Vector4 Colour = Vector4.One;
		public float Thickness = 1.0f;
		public float Fade = .005f;
	}

	public enum ProjectionType { Orthographic = 0, Perspective = 1 };

	public class CameraComponent : Component
	{
		public ProjectionType ProjectionType = ProjectionType.Orthographic;
		public float VerticalSize = 10.0f;
		public float NearPlane = -1.0f;
		public float FarPlane = 1.0f;
		public bool FixedAspectRatio = false;
	}

	public class ScriptComponent : Component
	{
		public string ScriptName = "";
	}

	public enum BodyType { Static = 0, Dynamic = 1, Kinematic = 2 };

	public class RigidBody2DComponent : Component
	{		
		public BodyType Type = BodyType.Static;
		public bool FixedRotation = false;
	}

	public class RectColliderComponent : Component
	{
		public Vector2 Offset = Vector2.Zero;
		public Vector2 HalfExtent = new Vector2(.5f);
		public float Density = 1.0f;
		public float Friction = .5f;
		public float Restitution = .5f;
		public float RestitutionThreshold = .5f;
	}

	public class CircleColliderComponent : Component
	{
		public Vector2 Offset = Vector2.Zero;
		public float Radius = .5f;
		public float Density = 1.0f;
		public float Friction = .5f;
		public float Restitution = .5f;
		public float RestitutionThreshold = .5f;
	}

}
