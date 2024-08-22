#include "zpch.h"
#include "SceneSerialiser.h"

#include "Entity.h"

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#include <fstream>


namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;

			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2) return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;

			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3) return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;

			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);

			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4) return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();

			return true;
		}
	};

}

namespace Zahra
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& vector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vector.x << vector.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& vector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vector.x << vector.y << vector.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& vector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vector.x << vector.y << vector.z << vector.w << YAML::EndSeq;
		return out;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SERIALISATION HELPERS
	static std::string CameraProjectionTypeToString(SceneCamera::ProjectionType type)
	{
		switch (type)
		{
		case SceneCamera::ProjectionType::Orthographic: return "Orthographic";
		case SceneCamera::ProjectionType::Perspective: return "Perspective";
		}
		Z_CORE_ASSERT(false, "Invalid ProjectionType");
		return "";
	}

	static SceneCamera::ProjectionType CameraProjectionTypeFromString(std::string str)
	{
		if (str == "Orthographic")
			return SceneCamera::ProjectionType::Orthographic;
		else if (str == "Perspective")
			return SceneCamera::ProjectionType::Perspective;

		Z_CORE_ASSERT(false, "Unrecognised input string");
		return SceneCamera::ProjectionType::Orthographic;
	}

	static std::string RigidBody2DTypeToString(RigidBody2DComponent::BodyType type)
	{
		switch (type)
		{
		case RigidBody2DComponent::BodyType::Static: return "Static";
		case RigidBody2DComponent::BodyType::Dynamic: return "Dynamic";
		case RigidBody2DComponent::BodyType::Kinematic: return "Kinematic";
		}
		Z_CORE_ASSERT(false, "Invalid BodyType");
		return "";
	}

	static RigidBody2DComponent::BodyType RigidBody2DTypeFromString(std::string str)
	{
		if (str == "Static")
			return RigidBody2DComponent::BodyType::Static;
		else if (str == "Dynamic")
			return RigidBody2DComponent::BodyType::Dynamic;
		else if (str == "Kinematic")
			return RigidBody2DComponent::BodyType::Kinematic;

		Z_CORE_ASSERT(false, "Unrecognised input string");
		return RigidBody2DComponent::BodyType::Static;
	}
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SceneSerialiser::SceneSerialiser(const Ref<Scene>& scene)
		: m_Scene(scene) {}

	static void SerialiseEntity(YAML::Emitter& out, Entity entity)
	{
		// TODO: learn about "reflection" so that I can understand
		// how to do this without having to remember to add new
		// stuff here for each new component type

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// GUID
		Z_CORE_ASSERT(entity.HasComponents<IDComponent>(), "All entities must have an IDComponent");
		uint64_t entityUUID = (uint64_t)entity.GetComponents<IDComponent>().ID;

		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entityUUID;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TAG (COMPONENT NAME)
		Z_CORE_ASSERT(entity.HasComponents<TagComponent>(), "All entities must have a TagComponent");
		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap;
		{
			auto& tag = entity.GetComponents<TagComponent>().Tag;
			Z_CORE_TRACE("Serialising entity with ID = {0}, name = {1}", entityUUID, tag);

			out << YAML::Key << "Tag" << YAML::Value << tag;
		}
		out << YAML::EndMap;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TRANSFORM
		Z_CORE_ASSERT(entity.HasComponents<TransformComponent>(), "All entities must have a TransformComponent");
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap;
		{
			auto& transform = entity.GetComponents<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
			out << YAML::Key << "EulerAngles" << YAML::Value << transform.EulerAngles;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
		}
		out << YAML::EndMap;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// OTHER COMPONENTS
		if (entity.HasComponents<SpriteComponent>())
		{
			out << YAML::Key << "SpriteComponent";
			out << YAML::BeginMap;
			{
				auto& sprite = entity.GetComponents<SpriteComponent>();
				out << YAML::Key << "Tint" << YAML::Value << sprite.Tint;
				// TODO: once we have an assets manager I need to serialise attached textures!
			}
			out << YAML::EndMap;
		}

		if (entity.HasComponents<CircleComponent>())
		{
			out << YAML::Key << "CircleComponent";
			out << YAML::BeginMap;
			{
				auto& circle = entity.GetComponents<CircleComponent>();
				out << YAML::Key << "Colour" << YAML::Value << circle.Colour;
				out << YAML::Key << "Thickness" << YAML::Value << circle.Thickness;
				out << YAML::Key << "Fade" << YAML::Value << circle.Fade;
			}
			out << YAML::EndMap;
		}

		if (entity.HasComponents<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			{
				auto& cameraComponent = entity.GetComponents<CameraComponent>();
				auto& camera = cameraComponent.Camera;
				out << YAML::Key << "Camera" << YAML::Value;
				out << YAML::BeginMap;
				{	
					out << YAML::Key << "ProjectionType" << YAML::Value << CameraProjectionTypeToString(camera.GetProjectionType());

					out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
					out << YAML::Key << "OrthographicNearClip" << YAML::Value << camera.GetOrthographicNearClip();
					out << YAML::Key << "OrthographicFarClip" << YAML::Value << camera.GetOrthographicFarClip();

					out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveFOV();
					out << YAML::Key << "PerspectiveNearClip" << YAML::Value << camera.GetPerspectiveNearClip();
					out << YAML::Key << "PerspectiveFarClip" << YAML::Value << camera.GetPerspectiveFarClip();

				}
				out << YAML::EndMap;
			
				out << YAML::Key << "Active" << YAML::Value << cameraComponent.Active;
				out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
			}
			out << YAML::EndMap;
		}

		if (entity.HasComponents<RigidBody2DComponent>())
		{
			out << YAML::Key << "RigidBody2DComponent";
			out << YAML::BeginMap;
			{
				auto& body = entity.GetComponents<RigidBody2DComponent>();
				out << YAML::Key << "Type" << YAML::Value << RigidBody2DTypeToString(body.Type);
				out << YAML::Key << "FixedRotation" << YAML::Value << body.FixedRotation;

			}
			out << YAML::EndMap;
		}

		if (entity.HasComponents<RectColliderComponent>())
		{
			out << YAML::Key << "RectColliderComponent";
			out << YAML::BeginMap;
			{
				auto& collider = entity.GetComponents<RectColliderComponent>();
				out << YAML::Key << "Offset" << YAML::Value << collider.Offset;
				out << YAML::Key << "HalfExtent" << YAML::Value << collider.HalfExtent;
				out << YAML::Key << "Density" << YAML::Value << collider.Density;
				out << YAML::Key << "Friction" << YAML::Value << collider.Friction;
				out << YAML::Key << "Restitution" << YAML::Value << collider.Restitution;
				out << YAML::Key << "RestitutionThreshold" << YAML::Value << collider.RestitutionThreshold;

			}
			out << YAML::EndMap;
		}

		if (entity.HasComponents<CircleColliderComponent>())
		{
			out << YAML::Key << "CircleColliderComponent";
			out << YAML::BeginMap;
			{
				auto& collider = entity.GetComponents<CircleColliderComponent>();
				out << YAML::Key << "Offset" << YAML::Value << collider.Offset;
				out << YAML::Key << "Radius" << YAML::Value << collider.Radius;
				out << YAML::Key << "Density" << YAML::Value << collider.Density;
				out << YAML::Key << "Friction" << YAML::Value << collider.Friction;
				out << YAML::Key << "Restitution" << YAML::Value << collider.Restitution;
				out << YAML::Key << "RestitutionThreshold" << YAML::Value << collider.RestitutionThreshold;

			}
			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	void SceneSerialiser::SerialiseYaml(const std::string& filepath)
	{
		std::string sceneName = m_Scene->GetName();
		Z_CORE_TRACE("Serialising scene '{0}'", sceneName);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << sceneName;
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.view<entt::entity>().each([&](auto entityHandle)
			{
				Entity entity = { entityHandle, m_Scene.get() };
				if (!entity) return;

				SerialiseEntity(out, entity);
			});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();

	}

	bool SceneSerialiser::DeserialiseYaml(const std::string& filepath)
	{
		YAML::Node data = YAML::LoadFile(filepath);
		if (!data["Scene"]) return false;

		std::string sceneName = data["Scene"].as<std::string>();
		m_Scene->SetName(sceneName);
		Z_CORE_TRACE("Deserialising scene '{0}'", sceneName);

		auto entityNodes = data["Entities"];
		if (entityNodes)
		{
			for (auto entityNode : entityNodes)
			{
				uint64_t uuid = entityNode["Entity"].as<uint64_t>();

				std::string name = "unnamed_entity";
				auto tagNode = entityNode["TagComponent"];
				if (tagNode) name = tagNode["Tag"].as<std::string>();

				Z_CORE_TRACE("Deserialising entity with ID = {0}, name = {1}", uuid, name);

				Entity entity = m_Scene->CreateEntity(uuid, name);

				auto transformNode = entityNode["TransformComponent"];
				if (transformNode)
				{
					// since every entity is given a transform on creation, we must use GetComponents instead of AddComponent here
					auto& transform = entity.GetComponents<TransformComponent>();

					transform.Translation = transformNode["Translation"].as<glm::vec3>();
					transform.EulerAngles = transformNode["EulerAngles"].as<glm::vec3>();
					transform.Scale = transformNode["Scale"].as<glm::vec3>();
				}

				auto spriteNode = entityNode["SpriteComponent"];
				if (spriteNode)
				{
					auto& sprite = entity.AddComponent<SpriteComponent>();

					sprite.Tint = spriteNode["Tint"].as<glm::vec4>();
					// TODO: textures etc.
				}

				auto circleNode = entityNode["CircleComponent"];
				if (circleNode)
				{
					auto& circle = entity.AddComponent<CircleComponent>();

					circle.Colour = circleNode["Colour"].as<glm::vec4>();
					circle.Thickness = circleNode["Thickness"].as<float>();
					circle.Fade = circleNode["Fade"].as<float>();
				}

				auto cameraNode = entityNode["CameraComponent"];
				if (cameraNode)
				{
					auto& camera = entity.AddComponent<CameraComponent>();

					camera.Active = cameraNode["Active"].as<bool>();
					camera.FixedAspectRatio = cameraNode["FixedAspectRatio"].as<bool>();

					auto& cameraSubnode = cameraNode["Camera"];
					
					camera.Camera.SetOrthographicData(
						cameraSubnode["OrthographicSize"].as<float>(),
						cameraSubnode["OrthographicNearClip"].as<float>(),
						cameraSubnode["OrthographicFarClip"].as<float>()
					);
					camera.Camera.SetPerspectiveData(
						cameraSubnode["PerspectiveFOV"].as<float>(),
						cameraSubnode["PerspectiveNearClip"].as<float>(),
						cameraSubnode["PerspectiveFarClip"].as<float>()
					);
					camera.Camera.SetProjectionType(CameraProjectionTypeFromString(cameraSubnode["ProjectionType"].as<std::string>()));

				}

				auto rigidBody2DNode = entityNode["RigidBody2DComponent"];
				if (rigidBody2DNode)
				{
					auto& body = entity.AddComponent<RigidBody2DComponent>();

					body.Type = RigidBody2DTypeFromString(rigidBody2DNode["Type"].as<std::string>());
					body.FixedRotation = rigidBody2DNode["FixedRotation"].as<bool>();
				}

				auto rectColliderNode = entityNode["RectColliderComponent"];
				if (rectColliderNode)
				{
					auto& collider = entity.AddComponent<RectColliderComponent>();

					collider.Offset = rectColliderNode["Offset"].as<glm::vec2>();
					collider.HalfExtent = rectColliderNode["HalfExtent"].as<glm::vec2>();
					collider.Density = rectColliderNode["Density"].as<float>();
					collider.Friction = rectColliderNode["Friction"].as<float>();
					collider.Restitution = rectColliderNode["Restitution"].as<float>();
					collider.RestitutionThreshold = rectColliderNode["RestitutionThreshold"].as<float>();
				}

				auto circleColliderNode = entityNode["CircleColliderComponent"];
				if (circleColliderNode)
				{
					auto& collider = entity.AddComponent<CircleColliderComponent>();

					collider.Offset = circleColliderNode["Offset"].as<glm::vec2>();
					collider.Radius = circleColliderNode["Radius"].as<float>();
					collider.Density = circleColliderNode["Density"].as<float>();
					collider.Friction = circleColliderNode["Friction"].as<float>();
					collider.Restitution = circleColliderNode["Restitution"].as<float>();
					collider.RestitutionThreshold = circleColliderNode["RestitutionThreshold"].as<float>();
				}

			}
		}

		return true;
	}

	void SceneSerialiser::SeraliseBin(const std::string& filepath)
	{
		// TODO: write this
		Z_CORE_ASSERT(false, "Method not implemented yet");
	}

	bool SceneSerialiser::DeseraliseBin(const std::string& filepath)
	{
		// TODO: write this
		Z_CORE_ASSERT(false, "Method not implemented yet");
		return false;
	}

}

