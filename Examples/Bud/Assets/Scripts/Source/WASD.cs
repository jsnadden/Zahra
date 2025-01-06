
using Djinn;

namespace Bud
{
	public class WASD : Entity
	{
		WASD() : base() {}
		WASD(ulong guid) : base(guid) {}

		public float power;

		public Vector2 vector2;
		public Entity entity;
		public char character;
		public Quaternion quat;

		private TransformComponent transformComponent;
		private RigidBody2DComponent rigidBody2DComponent;

		public void OnCreate()
		{
			transformComponent = GetComponent<TransformComponent>();
			rigidBody2DComponent = GetComponent<RigidBody2DComponent>();

			power = 10.0f;
		}

		public void OnUpdate(float dt)
		{
			/*float framerate = 1.0f / dt;
			Djinn.Zahra.Log_Trace($"Framerate: {framerate} fps");*/

			Vector2 force = Vector2.Zero;
			Vector2 impulse = Vector2.Zero;
			
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
				force.Y = 2.0f;
			}
			else if (Input.IsKeyDown(KeyCode.S))
			{
				force.Y = -2.0f;
			}

			//force.Normalise();
			force *= power;

			rigidBody2DComponent.ApplyForce(force, true);
			//rigidBody2DComponent.ApplyLinearImpulse(impulse * dt, true);
		}
	}
}
