#include "zpch.h"
#include "SceneSerialiser.h"

#include "Entity.h"

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#include <fstream>

namespace YAML
{

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

	SceneSerialiser::SceneSerialiser(const Ref<Scene>& scene)
		: m_Scene(scene) {}

	static void SerialiseEntity(YAML::Emitter& out, Entity entity)
	{
		// TODO: learn about "reflection" so that I can understand
		// how to do this without having to remember to add new
		// stuff here for each new component type

		uint64_t entityUUID = 23152837592538; // TODO: this should be the UUID for the entity

		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entityUUID;

		if (entity.HasComponents<TagComponent>()) // by all rights, it damn well should!
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			{
				auto& tag = entity.GetComponents<TagComponent>().Tag;
				Z_CORE_TRACE("Serialising entity with ID = {0}, name = {1}", entityUUID, tag);

				out << YAML::Key << "Tag" << YAML::Value << tag;
			}
			out << YAML::EndMap;
		}

		if (entity.HasComponents<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
			{
				auto& transform = entity.GetComponents<TransformComponent>();
				out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
				out << YAML::Key << "EulerAngles" << YAML::Value << transform.EulerAngles;
				out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
			}
			out << YAML::EndMap;
		}

		if (entity.HasComponents<SpriteComponent>())
		{
			out << YAML::Key << "SpriteComponent";
			out << YAML::BeginMap;
			{
				auto& sprite = entity.GetComponents<SpriteComponent>();
				out << YAML::Key << "Tint" << YAML::Value << sprite.Tint;
				// TODO: how to serialise a texture?!?!
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
					out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();

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
		m_Scene->m_Registry.view<entt::entity>().each([&](auto entityID)
			{
				Entity entity = { entityID, m_Scene.get() };
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
				uint64_t uuid = entityNode["Entity"].as<uint64_t>(); // TODO: return to this when we have our UUID system in place

				std::string name;
				auto tagNode = entityNode["TagComponent"];
				if (tagNode) name = tagNode["Tag"].as<std::string>();

				Z_CORE_TRACE("Deserialising entity with ID = {0}, name = {1}", uuid, name);

				Entity entity = m_Scene->CreateEntity(name); // TODO: ideally this would be an overloaded CreateEntity accepting a name AND a UUID

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
					camera.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraSubnode["ProjectionType"].as<int>());

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

