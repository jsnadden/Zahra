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
		: m_Scene(scene), m_Panel(panel) {}

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
		const ZGUID m_entityID;
		WeakRef<Scene> m_Scene;
		SceneHierarchyPanel& m_Panel;
	};

	class EntityDuplication : public Edit
	{
	public:
		EntityDuplication(ZGUID originalID, WeakRef<Scene> scene, SceneHierarchyPanel& panel)
		: m_OriginalID(originalID), m_Scene(scene), m_Panel(panel) {}

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
		const ZGUID m_OriginalID, m_DuplicateID;
		WeakRef<Scene> m_Scene;
		SceneHierarchyPanel& m_Panel;
	};

	class EntityDestruction : public Edit
	{
	public:
		EntityDestruction() {}

		// TODO: this is going to be a pain:
			//	Go through every possible component, recording whether the entity has one, and if so, its data.
			//	The do command is easy enough:
			//			1) just capture [&, GUID]
			//			2) deselect if entity == selection
			//			3) destroy entity using its GUID 
			//	The undo command is going to be more of a pain:
			//			1) capture the cached component and bool values i.e. [&, cachedT, hasT, ...]
			//			2) create entity using its previous GUID and Tag values
			//			3) for each component type T: if hasT, then add a T and set it equal to cachedT

		virtual void Do() override
		{

		}

		virtual void Undo() override
		{

		}

	private:

	};
	
	class ComponentAddition : public Edit
	{
	public:
		ComponentAddition() {}

		virtual void Do() override
		{

		}

		virtual void Undo() override
		{

		}

	private:

	};

	class ComponentDeletion : public Edit
	{
	public:
		ComponentDeletion() {}

		virtual void Do() override
		{

		}

		virtual void Undo() override
		{

		}

	private:

	};

	template <typename T>
	class ValueEdit : public Edit
	{
	public:
		ValueEdit(T& value, T from, T to)
		: m_Value(value), m_From(from), m_To(to) {}

		virtual void Do() override
		{
			m_Value = m_To;
		}

		virtual void Undo() override
		{
			m_Value = m_From;
		}

	private:
		T& m_Value;
		const T m_From, m_To;
	};

	class EulerAngleEdit : public Edit
	{
	public:
		EulerAngleEdit(TransformComponent& transform, glm::vec3 from, glm::vec3 to)
			: m_Transform(transform), m_From(from), m_To(to) {}

		virtual void Do() override
		{
			m_Transform.SetRotation(m_To);
		}

		virtual void Undo() override
		{
			m_Transform.SetRotation(m_From);
		}

	private:
		TransformComponent& m_Transform;
		const glm::vec3 m_From, m_To;
	};

	class GizmoManipulation : public Edit
	{
	public:
		GizmoManipulation(TransformComponent& transform, glm::vec3 from, glm::vec3 to, TransformationType type)
			: m_Transform(transform), m_From(from), m_To(to), m_Type(type) {}

		virtual void Do() override
		{
			switch (m_Type)
			{
				case TransformationType::Translation:	m_Transform.Translation = m_To;		break;
				case TransformationType::Rotation:		m_Transform.SetRotation(m_To);		break;
				case TransformationType::Scale:			m_Transform.Scale = m_To;			break;
			}
		}

		virtual void Undo() override
		{
			switch (m_Type)
			{
				case TransformationType::Translation:	m_Transform.Translation = m_From;	break;
				case TransformationType::Rotation:		m_Transform.SetRotation(m_From);	break;
				case TransformationType::Scale:			m_Transform.Scale = m_From;			break;
			}
		}

	private:
		TransformComponent& m_Transform;
		const glm::vec3 m_From, m_To;
		const TransformationType m_Type;
	};
}
