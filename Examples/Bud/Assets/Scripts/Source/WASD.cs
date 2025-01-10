
using Djinn;
using System.Reflection;

namespace Bud
{
	public class WASD : Entity
	{
		WASD() : base() {}
		WASD(ulong guid) : base(guid) { }

		public float Power = 50.0f;
		public float Resistance = 1.0f;
		public bool AntiGravity = false;

		public void OnCreate()
		{
			transformComponent = GetComponent<TransformComponent>();
			rigidBody2DComponent = GetComponent<RigidBody2DComponent>();
		}

		public void OnEarlyUpdate(float dt)
		{
			Vector2 force = Vector2.Zero;
			
			if (Input.IsKeyDown(KeyCode.A))
			{
				force.X = -1.0f;
			}
			else if (Input.IsKeyDown(KeyCode.D))
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
			force.Y *= 2.0f;
			force *= Power;

			float mass = .0f;
			if (HasComponent<CircleColliderComponent>())
			{
				float radius = GetComponent<CircleColliderComponent>().Radius;
				float density = GetComponent<CircleColliderComponent>().Density;
				mass = 3.1415f * radius * radius * density;
			}

			if (AntiGravity)
				force.Y += 9.8f * mass;

			Vector2 velocity = rigidBody2DComponent.GetVelocity();
			force += velocity * -velocity.Norm() * Resistance;

			rigidBody2DComponent.ApplyForce(force, true);
		}

		public void OnLateUpdate(float dt)
		{
			
		}

		private TransformComponent transformComponent;
		private RigidBody2DComponent rigidBody2DComponent;

	}
}
