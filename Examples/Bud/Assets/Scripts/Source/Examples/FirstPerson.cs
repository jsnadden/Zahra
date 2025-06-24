using Djinn;
using Djinn.CustomAttributes;
using System;

namespace Bud.Examples
{
	public class FirstPerson : Entity
	{
		public FirstPerson() : base() {}
		public FirstPerson(ulong uuid) : base(uuid) { }

		private TransformComponent transform;
		private RigidBody2DComponent body;
		private CameraComponent camera;

		[ExposedField] private float InputStrength_Linear;
		[ExposedField] private float InputStrength_Rotation;
		[ExposedField] public float Drag_Linear;
		[ExposedField] public float Drag_Rotation;
		
		public void OnCreate()
		{
			transform = GetComponent<TransformComponent>();
			body = GetComponent<RigidBody2DComponent>();
			camera = GetComponent<CameraComponent>();

			transform.Translation = new Vector3(0.0f, -5.0f, 1.0f);
			transform.EulerAngles = new Vector3(0.5f * (float)Math.PI, 0.0f, 0.0f);
		}

		// The OnEarlyUpdate method will be called each frame, before the physics engine is updated
		public void OnEarlyUpdate(float dt)
		{
			// Compute mass and moment of inertia
			float mass = .0f;
			float moment = .0f;
			if (HasComponent<CircleColliderComponent>())
			{
				float density = GetComponent<CircleColliderComponent>().Density;
				mass = density * (float)Math.PI * .25f * transform.Scale.X * transform.Scale.Y;
				moment = .0625f * mass * (float)(Math.Pow(transform.Scale.X, 2) + Math.Pow(transform.Scale.Y, 2));
			}
			else if (HasComponent<RectColliderComponent>())
			{
				float density = GetComponent<RectColliderComponent>().Density;
				mass = density * transform.Scale.X * transform.Scale.Y;
				moment = .08333f * mass * (float)(Math.Pow(transform.Scale.X, 2) + Math.Pow(transform.Scale.Y, 2));

			}

			// TODO: thresholds for norms of velocity/angular velocity, below which they get set to zero

			// Handle movement input
			Vector3 force = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.Q))
			{
				force.X = -1.0f;
			}
			else if (Input.IsKeyDown(KeyCode.E))
			{
				force.X = 1.0f;
			}

			if (Input.IsKeyDown(KeyCode.W))
			{
				force.Y = 1.0f;
			}
			else if (Input.IsKeyDown(KeyCode.S))
			{
				force.Y = -1.0f;
			}

			force.Normalise();
			force *= InputStrength_Linear;

			Vector3 direction = new Vector3(0,0,transform.EulerAngles.Z);
			Quaternion rotation = new Quaternion(direction);
			force = rotation.Rotate(force);

			Vector2 force2 = force.XY;

			Vector2 velocity = body.GetVelocity();
			force2 += velocity * velocity.Norm() * Drag_Linear * -1.0f;

			body.ApplyForce(force2, true);

			// Handle rotation input
			float torque = 0.0f;

			if (Input.IsKeyDown(KeyCode.A))
			{
				torque = 1.0f;
			}
			else if (Input.IsKeyDown(KeyCode.D))
			{
				torque = -1.0f;
			}

			torque *= InputStrength_Rotation;

			float angularVelocity = body.GetAngularVelocity();
			torque += angularVelocity * Math.Abs(angularVelocity) * Drag_Rotation * -1.0f;

			body.ApplyTorque(torque, true);
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
