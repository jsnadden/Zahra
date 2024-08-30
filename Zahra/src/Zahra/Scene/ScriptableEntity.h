#pragma once

#include "Entity.h"

namespace Zahra
{

	class NativeScriptableEntity
	{
	public:
		virtual ~NativeScriptableEntity() {}

		template<typename ...Types>
		auto& GetComponents()
		{
			return m_Entity.GetComponents<Types...>();
		}

		template<typename ...Types>
		bool HasComponents()
		{
			return m_Entity.HasComponents<Types...>();
		}

	protected:
		virtual void OnCreate() {};
		virtual void OnDestroy() {};
		virtual void OnUpdate(float dt) {};

	private:
		Entity m_Entity;

		friend class Scene;
	};

	// e.g.
	class CameraController : public NativeScriptableEntity
		{
		public:
			void OnUpdate(float dt)
			{
				auto& position = GetComponents<TransformComponent>().Translation;
				if (HasComponents<CameraComponent>())
				{
					auto& camera = GetComponents<CameraComponent>().Camera;
					float speed = camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic
						? .5f * camera.GetOrthographicSize() : 2.0f;
		
					if (Input::IsKeyPressed(KeyCode::A))
						position.x -= speed * dt;
					if (Input::IsKeyPressed(KeyCode::D))
						position.x += speed * dt;
					if (Input::IsKeyPressed(KeyCode::W))
						position.y += speed * dt;
					if (Input::IsKeyPressed(KeyCode::S))
						position.y -= speed * dt;
				}
			}
		};


}
