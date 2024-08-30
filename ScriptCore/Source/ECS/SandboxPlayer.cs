
using System;
using Zahra;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Sandbox
{
	public class Player : Zahra.Entity
	{

		Player() : base() {}
		Player(ulong guid) : base(guid) {}


		public void OnCreate() 
		{
			
		}

		public void OnUpdate(float dt)
		{
			/*float framerate = 1.0f / dt;
			InternalCalls.NativeLog($"Framerate: {framerate} fps");*/

			float speed = 10.0f;
			Vector3 velocity = Vector3.Zero;

			if (InternalCalls.Input_IsKeyDown(KeyCode.W))
			{
				velocity.Y = 1.0f;
			}
			else if (InternalCalls.Input_IsKeyDown(KeyCode.S))
			{
				velocity.Y = -1.0f;
			}
			
			if (InternalCalls.Input_IsKeyDown(KeyCode.A))
			{
				velocity.X = -1.0f;
			}
			else if (InternalCalls.Input_IsKeyDown(KeyCode.D))
			{
				velocity.X = 1.0f;
			}

			velocity.Normalise();
			velocity *= speed;

			Vector3 translation = Translation;
			translation += velocity * dt;
			Translation = translation;
		}
	}
}
