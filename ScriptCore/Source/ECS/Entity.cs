using System;
using System.Runtime.CompilerServices;

namespace Zahra
{

	public class Entity
	{
		protected Entity() { GUID = 0; Console.WriteLine("guidless entity constructor"); }
		internal Entity(ulong guid) { GUID = guid; Console.WriteLine("guidful entity constructor"); }

		public readonly ulong GUID;

		public Vector3 Translation
		{
			get
			{
				InternalCalls.Entity_GetTranslation(GUID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.Entity_SetTranslation(GUID, ref value);
			}

		}

	}

}

