
using System;
using Zahra;

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

			float speed = 20.0f;

			Vector3 translation = Translation;
			translation.Y += dt * speed;
			Translation = translation;
			//Vector3 translation = Translation;
			/*translation += speed * new Vector3(1,0,0);
			Translation = translation;*/
		}
	}
}
