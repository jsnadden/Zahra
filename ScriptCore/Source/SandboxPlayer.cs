using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sandbox
{
	public class Player : Zahra.Entity
	{
		void OnCreate()
		{
			Zahra.InternalCalls.NativeLog("Calling Player.OnCreate()");
		}
		void OnUpdate(float dt)
		{
			Zahra.InternalCalls.NativeLog($"Calling Player.OnUpdate({dt})");
		}
	}
}
