using System;

namespace Djinn
{

	public class Entity
	{
		protected Entity() { GUID = 0; }
		internal Entity(ulong guid) { GUID = guid; }

		public readonly ulong GUID;

		public Vector3 Translation
		{
			get
			{
				Vector3 translation = Vector3.Zero;
				Zahra.TransformComponent_GetTranslation(GUID, out translation);
				return translation;
			}
			set
			{
				Zahra.TransformComponent_SetTranslation(GUID, ref value);
			}
		}

	}

}

