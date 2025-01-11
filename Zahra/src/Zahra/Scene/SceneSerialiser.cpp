#include "zpch.h"
#include "SceneSerialiser.h"

#include "Zahra/Scene/Entity.h"
#include "Zahra/Scripting/ScriptEngine.h"

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

	static void SerialiseEntity(YAML::Emitter& out, Entity entity, Ref<Scene> scene)
	{
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// GUID
		Z_CORE_ASSERT(entity.HasComponents<IDComponent>(), "All entities must have an IDComponent");
		uint64_t entityGUID = (uint64_t)entity.GetComponents<IDComponent>().ID;

		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entityGUID;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TAG (COMPONENT NAME)
		Z_CORE_ASSERT(entity.HasComponents<TagComponent>(), "All entities must have a TagComponent");
		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap;
		{
			auto& tag = entity.GetComponents<TagComponent>().Tag;
			//Z_CORE_TRACE("Serialising entity {0} (GUID = {1})", tag, entityGUID);

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
			out << YAML::Key << "EulerAngles" << YAML::Value << transform.GetEulers();
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
				if (sprite.Texture)
				{
					// TODO: serialise texture as asset GUID instead
					std::string texturePath = sprite.Texture->GetFilepath().string();
					out << YAML::Key << "TexturePath" << YAML::Value << texturePath.c_str();
				}
				out << YAML::Key << "TextureTiling" << YAML::Value << sprite.TextureTiling;
				out << YAML::Key << "Animated" << YAML::Value << sprite.Animated;
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
			
				//out << YAML::Key << "Active" << YAML::Value << cameraComponent.Active;
				out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
			}
			out << YAML::EndMap;
		}

		if (entity.HasComponents<ScriptComponent>())
		{
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap;
			{
				auto& script = entity.GetComponents<ScriptComponent>();
				out << YAML::Key << "ScriptName" << YAML::Value << script.ScriptName;

				out << YAML::Key << "FieldValues";
				out << YAML::BeginMap;
				{
					auto fields = ScriptEngine::GetScriptClassIfValid(script.ScriptName)->GetPublicFields();
					auto buffer = scene->GetScriptFieldStorage(entity);

					for (uint64_t i = 0; i < fields.size(); i++)
					{
						auto& field = fields[i];
						uint64_t offset = 8 * i;
						
						switch (field.Type)
						{
							case ScriptFieldType::Bool:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<bool>(offset);
								break;
							}
							case ScriptFieldType::sByte:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<int8_t>(offset);
								break;
							}
							case ScriptFieldType::Byte:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<uint8_t>(offset);
								break;
							}
							case ScriptFieldType::Short:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<int16_t>(offset);
								break;
							}
							case ScriptFieldType::uShort:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<uint16_t>(offset);
								break;
							}
							case ScriptFieldType::Char:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<uint16_t>(offset);
								break;
							}
							case ScriptFieldType::Int:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<int32_t>(offset);
								break;
							}
							case ScriptFieldType::uInt:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<uint32_t>(offset);
								break;
							}
							case ScriptFieldType::Long:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<int64_t>(offset);
								break;
							}
							case ScriptFieldType::uLong:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<uint64_t>(offset);
								break;
							}
							case ScriptFieldType::Float:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<float>(offset);
								break;
							}
							case ScriptFieldType::Double:
							{
								out << YAML::Key << field.Name << YAML::Value << buffer.ReadAs<double>(offset);
								break;
							}
							// TODO: include Djinn types
						}
					}
				}
				out << YAML::EndMap;
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
		//Z_CORE_TRACE("Serialising scene '{0}'", sceneName);

		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Scene";
			out << YAML::Value << sceneName;

			if (Entity activeCamera = m_Scene->GetActiveCamera())
			{
				out << YAML::Key << "ActiveCameraGUID";
				out << YAML::Value << activeCamera.GetGUID();
			}

			out << YAML::Key << "Entities";
			out << YAML::Value << YAML::BeginSeq;
			{
				m_Scene->m_Registry.sort<entt::entity>([](const auto& lhs, const auto& rhs) { return lhs < rhs; });
				m_Scene->m_Registry.view<entt::entity>().each([&](auto entityHandle)
					{
						Entity entity = { entityHandle, m_Scene.Raw() };
						if (!entity) return;

						SerialiseEntity(out, entity, m_Scene);
					});
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();

	}

	bool SceneSerialiser::DeserialiseYaml(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (const YAML::ParserException& ex)
		{
			Z_CORE_ERROR("Failed to deserialize scene '{0}'\n     {1}", filepath, ex.what());
			return false;
		}

		if (!data["Scene"]) return false;

		std::string sceneName = data["Scene"].as<std::string>();
		m_Scene->SetName(sceneName);
		//Z_CORE_TRACE("Deserialising scene '{0}'", sceneName);

		bool hasActiveCamera = (bool)data["ActiveCameraGUID"];
		uint64_t cameraGUID;
		if (hasActiveCamera)
			cameraGUID = data["ActiveCameraGUID"].as<uint64_t>();

		Texture2DSpecification textureSpec{};
		if (Application::Get().GetSpecification().ImGuiConfig.ColourCorrectSceneTextures)
			textureSpec.Format = ImageFormat::RGBA_UN;

		auto entityNodes = data["Entities"];
		if (entityNodes)
		{
			for (auto entityNode : entityNodes)
			{
				uint64_t entityGUID = entityNode["Entity"].as<uint64_t>();

				std::string tag = "unnamed_entity";
				auto tagNode = entityNode["TagComponent"];
				if (tagNode) tag = tagNode["Tag"].as<std::string>();

				//Z_CORE_TRACE("Deserialising entity {0} (GUID = {1})", tag, entityGUID);

				Entity entity = m_Scene->CreateEntity(entityGUID, tag);

				auto transformNode = entityNode["TransformComponent"];
				if (transformNode)
				{
					// since every entity is given a transform on creation, we must use GetComponents instead of AddComponent here
					auto& transform = entity.GetComponents<TransformComponent>();

					transform.Translation = transformNode["Translation"].as<glm::vec3>();
					transform.SetRotation(transformNode["EulerAngles"].as<glm::vec3>());
					transform.Scale = transformNode["Scale"].as<glm::vec3>();
				}

				auto spriteNode = entityNode["SpriteComponent"];
				if (spriteNode)
				{
					auto& sprite = entity.AddComponent<SpriteComponent>();

					sprite.Tint = spriteNode["Tint"].as<glm::vec4>();
					if (spriteNode["TexturePath"])
						sprite.Texture = Texture2D::CreateFromFile(textureSpec, spriteNode["TexturePath"].as<std::string>());
					sprite.TextureTiling = spriteNode["TextureTiling"].as<float>();
					sprite.Animated = spriteNode["Animated"].as<bool>();
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

					//camera.Active = cameraNode["Active"].as<bool>();
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

				if (hasActiveCamera)
				{
					if (cameraGUID == entityGUID)
						m_Scene->SetActiveCamera(entity);
				}

				auto scriptNode = entityNode["ScriptComponent"];
				if (scriptNode)
				{
					auto& script = entity.AddComponent<ScriptComponent>(scriptNode["ScriptName"].as<std::string>());

					auto fieldNodes = scriptNode["FieldValues"];
					auto scriptClass = ScriptEngine::GetScriptClassIfValid(script.ScriptName);

					if (fieldNodes && scriptClass)
					{
						auto fields = scriptClass->GetPublicFields();
						auto buffer = m_Scene->GetScriptFieldStorage(entity);

						for (uint64_t i = 0; i < fields.size(); i++)
						{
							auto& field = fields[i];
							uint64_t offset = 8 * i;

							auto fieldNode = fieldNodes[field.Name];
							if (!fieldNode)
								continue;

							switch (field.Type)
							{
								case ScriptFieldType::Bool:
								{
									bool value = fieldNode.as<bool>();
									buffer.Write((void*)&value, sizeof(bool), offset);
									break;
								}
								case ScriptFieldType::sByte:
								{
									int8_t value = fieldNode.as<int8_t>();
									buffer.Write((void*)&value, sizeof(int8_t), offset);
									break;
								}
								case ScriptFieldType::Byte:
								{
									uint8_t value = fieldNode.as<uint8_t>();
									buffer.Write((void*)&value, sizeof(uint8_t), offset);
									break;
								}
								case ScriptFieldType::Short:
								{
									int16_t value = fieldNode.as<int16_t>();
									buffer.Write((void*)&value, sizeof(int16_t), offset);
									break;
								}
								case ScriptFieldType::uShort:
								{
									uint16_t value = fieldNode.as<uint16_t>();
									buffer.Write((void*)&value, sizeof(uint16_t), offset);
									break;
								}
								case ScriptFieldType::Char:
								{
									uint16_t value = fieldNode.as<uint16_t>();
									buffer.Write((void*)&value, sizeof(uint16_t), offset);
									break;
								}
								case ScriptFieldType::Int:
								{
									int32_t value = fieldNode.as<int32_t>();
									buffer.Write((void*)&value, sizeof(int32_t), offset);
									break;
								}
								case ScriptFieldType::uInt:
								{
									uint32_t value = fieldNode.as<uint32_t>();
									buffer.Write((void*)&value, sizeof(uint32_t), offset);
									break;
								}
								case ScriptFieldType::Long:
								{
									int64_t value = fieldNode.as<int64_t>();
									buffer.Write((void*)&value, sizeof(int64_t), offset);
									break;
								}
								case ScriptFieldType::uLong:
								{
									uint64_t value = fieldNode.as<uint64_t>();
									buffer.Write((void*)&value, sizeof(uint64_t), offset);
									break;
								}
								case ScriptFieldType::Float:
								{
									float value = fieldNode.as<float>();
									buffer.Write((void*)&value, sizeof(float), offset);
									break;
								}
								case ScriptFieldType::Double:
								{
									double value = fieldNode.as<double>();
									buffer.Write((void*)&value, sizeof(double), offset);
									break;
								}
								// TODO: include Djinn types
							}
						}
					}
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

