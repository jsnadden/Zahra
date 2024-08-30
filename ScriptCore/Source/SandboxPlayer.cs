using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Zahra;

namespace Sandbox
{
	public class Player : Zahra.Entity
	{

		Player() : base() {}
		override public void OnCreate() 
		{
			InternalCalls.NativeLog("Hello, joe");
		}
		override public void OnUpdate(float dt)
		{
			
		}
	}
}
