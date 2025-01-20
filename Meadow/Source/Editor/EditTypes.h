#pragma once

#include "Editor/Editor.h"
#include "Utils/TypeDefs.h"
#include "Panels/SceneHierarchyPanel.h"

namespace Zahra
{
	class EntityCreation : public Edit
	{
	public:
		EntityCreation(WeakRef<Scene> scene, SceneHierarchyPanel& panel)
			: m_Scene(scene), m_Panel(panel) {
		}

		virtual void Do() override
		{
			m_Scene->CreateEntity(m_entityID);
		}

		virtual void Undo() override
		{
			if (m_Panel.IsSelected(m_entityID))
				m_Panel.Deselect();

			m_Scene->DestroyEntity(m_entityID);
		}

	private:
		const UUID m_entityID;
		WeakRef<Scene> m_Scene;
		SceneHierarchyPanel& m_Panel;
	};

	class EntityDuplication : public Edit
	{
	public:
		EntityDuplication(UUID originalID, WeakRef<Scene> scene, SceneHierarchyPanel& panel)
			: m_OriginalID(originalID), m_Scene(scene), m_Panel(panel) {
		}

		virtual void Do() override
		{
			Entity original = m_Scene->GetEntity(m_OriginalID);
			m_Scene->DuplicateEntity(original, m_DuplicateID);
		}

		virtual void Undo() override
		{
			if (m_Panel.IsSelected(m_DuplicateID))
				m_Panel.Deselect();

			m_Scene->DestroyEntity(m_DuplicateID);
		}

	private:
		const UUID m_OriginalID, m_DuplicateID;
		WeakRef<Scene> m_Scene;
		SceneHierarchyPanel& m_Panel;
	};

	// NOTE: component structs are intended to be small, as that lends itself to cache-friendliness
	// in the ECS iteration scheme. We can take advantage of this fact here, since it means that
	// instead of caching the values of individual struct members, we can just copy the entire struct,
	// and not have to worry too much about this hogging memory

	class EntityDestruction : public Edit
	{
	public:
		EntityDestruction() {}

		// TODO: this is going to be a pain:
			//	Go through every possible component, recording whether the entity has one, and if so, its data.
			//	The do command is easy enough:
			//			1) just capture [&, UUID]
			//			2) deselect if entity == selection
			//			3) destroy entity using its UUID 
			//	The undo command is going to be more of a pain:
			//			1) capture the cached component and bool values i.e. [&, cachedT, hasT, ...]
			//			2) create entity using its previous UUID and Tag values
			//			3) for each component type T: if hasT, then add a T and set it equal to cachedT

		virtual void Do() override
		{

		}

		virtual void Undo() override
		{

		}

	private:
		const UUID m_EntityID;

		// option a) do some variadic template wizardry (using MostComponents) to define a struct
		// (or buffer?) which caches all component values, plus the corresponding existence bool
	};

	template <typename Component>
	class ComponentAddition : public Edit
	{
	public:
		ComponentAddition(UUID entityID)
		: m_EntityID(entityID) {}

		virtual void Do() override
		{

		}

		virtual void Undo() override
		{

		}

	private:
		const UUID m_EntityID;

	};

	template <typename Component>
	class ComponentDeletion : public Edit
	{
	public:
		ComponentDeletion(UUID entityID, Component value)
			: m_EntityID(entityID), m_Value(value) {}

		virtual void Do() override
		{

		}

		virtual void Undo() override
		{

		}

	private:
		const UUID m_EntityID;

		// store value of component prior to deletion
		const Component m_Value;
	};

	template <typename Component>
	class ComponentValueEdit : public Edit
	{
	public:
		ComponentValueEdit(UUID entityID, Component before, Component after)
			: m_EntityID(entityID), m_Before(before), m_After(after) {}

		virtual void Do() override
		{

		}

		virtual void Undo() override
		{

		}

	private:
		const UUID m_EntityID;

		// store value of component before/after change
		const Component m_Before, m_After;
	};
}
