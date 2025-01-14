using Djinn;
using Djinn.CustomAttributes;
using System;

namespace Bud.Examples
{
	public class Player : Entity
	{
		public Player() : base() {}
		public Player(ulong guid) : base(guid) { }

		private TransformComponent transform;
		private RigidBody2DComponent body;

		// The [ExposedField] attribute signals to Zahra's ScriptEngine that these
		// fields can be accessed and initialised (at script instantiation)
		[ExposedField] private float InputStrength;
		[ExposedField] public float Drag;
		[ExposedField] public bool IgnoreGravity;
		[ExposedField] public bool WASD;
		
		// The OnCreate method will be called after script instantiation
		public void OnCreate()
		{
			transform = GetComponent<TransformComponent>();
			body = GetComponent<RigidBody2DComponent>();
		}

		// The OnEarlyUpdate method will be called each frame, before the physics engine is updated
		public void OnEarlyUpdate(float dt)
		{
			Vector2 force = Vector2.Zero;
			
			if (Input.IsKeyDown(WASD ? KeyCode.A : KeyCode.Left))
			{
				force.X = -1.0f;
			}
			else if (Input.IsKeyDown(WASD ? KeyCode.D : KeyCode.Right))
			{
				force.X = 1.0f;
			}

			if (Input.IsKeyDown(WASD ? KeyCode.W : KeyCode.Up))
			{
				force.Y = 1.0f;
			}
			else if (Input.IsKeyDown(WASD ? KeyCode.S : KeyCode.Down))
			{
				force.Y = -1.0f;
			}

			force.Normalise();
			force.Y *= 2.0f;
			force *= InputStrength;

			float mass = .0f;
			if (HasComponent<CircleColliderComponent>())
			{
				float density = GetComponent<CircleColliderComponent>().Density;
				mass = density * 3.141592f * .25f * transform.Scale.X * transform.Scale.Y;
			}
			else if (HasComponent<RectColliderComponent>())
			{
				float density = GetComponent<RectColliderComponent>().Density;
				mass = density * transform.Scale.X * transform.Scale.Y;
			}

			if (IgnoreGravity)
				force.Y += 9.8f * mass;

			Vector2 velocity = body.GetVelocity();
			force += velocity * -velocity.Norm() * Drag;

			body.ApplyForce(force, true);
		}

		// The OnLateUpdate method will be called each frame, after the physics engine is updated
		public void OnLateUpdate(float dt)
		{
			
		}

		public Vector3 Position
		{
			get
			{
				return transform.Translation;
			}
			set
			{}
		}

	}
}
