using Djinn;
using Djinn.CustomAttributes;
using System;

namespace Bud
{
	public class WASD : Entity
	{
		WASD() : base() {}
		WASD(ulong guid) : base(guid) { }

		private TransformComponent transformComponent;
		private RigidBody2DComponent rigidBody2DComponent;

		// The [ExposedField] attribute signals to Zahra's ScriptEngine that these
		// fields can be accessed and initialised (at script instantiation)
		[ExposedField]private float InputForce = 50.0f;
		[ExposedField] public float DragCoeff = 1.0f;
		[ExposedField] public bool IgnoreGravity = false;

		// The [EntityID] attribute signals to Zahra's ScriptEngine that a ulong (64-bit
		// unsigned integer) should be treated as an entity ID for the purposes of editing
		[ExposedField, EntityID] public ulong buddy = 0;
		
		// The OnCreate method will be called after script instantiation
		public void OnCreate()
		{
			transformComponent = GetComponent<TransformComponent>();
			rigidBody2DComponent = GetComponent<RigidBody2DComponent>();

			buddy = FindEntityWithName("block").GUID;
			string buddyID = string.Format("My buddy has ID 0x{0:X}", buddy);
			Log.Info(buddyID);
		}

		// The OnEarlyUpdate method will be called each frame, before the physics engine is updated
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
			force *= InputForce;

			float mass = .0f;
			if (HasComponent<CircleColliderComponent>())
			{
				float density = GetComponent<CircleColliderComponent>().Density;
				mass = density * 3.141592f * .25f * transformComponent.Scale.X * transformComponent.Scale.Y;
			}
			else if (HasComponent<RectColliderComponent>())
			{
				float density = GetComponent<RectColliderComponent>().Density;
				mass = density * transformComponent.Scale.X * transformComponent.Scale.Y;
			}

			if (IgnoreGravity)
				force.Y += 9.8f * mass;

			Vector2 velocity = rigidBody2DComponent.GetVelocity();
			force += velocity * -velocity.Norm() * DragCoeff;

			rigidBody2DComponent.ApplyForce(force, true);
		}

		// The OnLateUpdate method will be called each frame, after the physics engine is updated
		public void OnLateUpdate(float dt)
		{
			
		}

	}
}
