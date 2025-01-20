using System;

namespace Djinn
{

	public class Entity
	{
		protected Entity() { UUID = 0; }
		public Entity(ulong uuid) { UUID = uuid; }

		// TODO: get rid of this eventually, it's just for testing the script system
		public Entity(string name)
		{
			UUID = Zahra.Entity_FindEntityByName(name);
		}

		public static implicit operator bool(Entity entity) => entity.UUID != 0;

		public readonly ulong UUID;

		public string Name
		{
			get
			{
				return Zahra.Entity_GetName(UUID);
			}
			set
			{}
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			return Zahra.Entity_HasComponent(UUID, componentType);
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

		public T As<T>() where T : Entity, new()
		{
			object instance = Zahra.Entity_GetScriptInstance(UUID);
			return instance as T;
		}

	}

}

