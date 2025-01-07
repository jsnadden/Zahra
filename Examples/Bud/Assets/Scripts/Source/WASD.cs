
using Djinn;

namespace Bud
{
	public class WASD : Entity
	{
		WASD() : base() {}
		WASD(ulong guid) : base(guid) {}

		public void OnCreate()
		{
			transformComponent = GetComponent<TransformComponent>();
			rigidBody2DComponent = GetComponent<RigidBody2DComponent>();

			power = 100.0f;
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
			force *= power;

			rigidBody2DComponent.ApplyForce(force, true);
		}

		public void OnLateUpdate(float dt)
		{

		}

		public float power;

		public Vector2 vector2;
		public Entity entity;
		public char character;
		public Quaternion quat;

		private TransformComponent transformComponent;
		private RigidBody2DComponent rigidBody2DComponent;

	}
}
