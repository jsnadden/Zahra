
using Djinn;

namespace Sandbox
{
	public class WASD : Djinn.Entity
	{
		WASD() : base() {}
		WASD(ulong guid) : base(guid) {}

		private TransformComponent transformComponent;
		private RigidBody2DComponent rigidBody2DComponent;

		public void OnCreate()
		{
			transformComponent = GetComponent<TransformComponent>();
			rigidBody2DComponent = GetComponent<RigidBody2DComponent>();
		}

		public void OnUpdate(float dt)
		{
			/*float framerate = 1.0f / dt;
			Djinn.Zahra.Log_Trace($"Framerate: {framerate} fps");*/

			Vector2 impulse = Vector2.Zero;

			if (Zahra.Input_IsKeyDown(KeyCode.W))
			{
				impulse.Y = 1.0f;
			}
			else if (Zahra.Input_IsKeyDown(KeyCode.S))
			{
				impulse.Y = -1.0f;
			}
			
			if (Zahra.Input_IsKeyDown(KeyCode.A))
			{
				impulse.X = -1.0f;
			}
			else if (Zahra.Input_IsKeyDown(KeyCode.D))
			{
				impulse.X = 1.0f;
			}

			//impulse.Normalise();
			impulse *= .05f;

			rigidBody2DComponent.ApplyLinearImpulse(impulse, true);
			//transformComponent.Translation += impulse * dt;
		}
	}
}
