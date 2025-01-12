using System;

namespace Djinn
{

	public class Entity
	{
		protected Entity() { GUID = 0; }
		protected Entity(ulong guid) { GUID = guid; }

		public static implicit operator bool(Entity entity) => entity.GUID != 0;

		public readonly ulong GUID;

		public string Name
		{
			get
			{
				return Zahra.Entity_GetName(GUID);
			}
			set
			{}
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			return Zahra.Entity_HasComponent(GUID, componentType);
		}

		public T GetComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
				return null;

			// TODO: should have a cache of created components that we can search here,
			// so we're not just creating new ones EVERY time this is called
			T component =  new T() { Entity = this };
			return component;

		}

		// TODO: get rid of this eventually, it's just for testing
		public Entity FindEntityWithName(string name)
		{
			ulong guid = Zahra.Entity_FindEntityByName(name);			
			return new Entity(guid);
		}

	}

}

