#include "zpch.h"
#include "EditorAssetManager.h"

#include "Zahra/Assets/AssetLoader.h"
#include "Zahra/Projects/Project.h"

#include <yaml-cpp/yaml.h>

namespace Zahra
{
	static const AssetMetadata s_NullMetadata;

	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) const
	{
		const auto& metadata = GetMetadata(handle);
		if (!metadata) // i.e. if not found in registry
			return nullptr;

		Ref<Asset> asset;

		if (IsAssetLoaded(handle))
		{
			asset = m_LoadedAssets.at(handle);
		}
		else
		{
			asset = AssetLoader::LoadAssetFromSource(handle, metadata);

			if (!asset)
				Z_CORE_ERROR("EditorAssetManager failed to load asset '{}'", metadata.Filepath.string().c_str());
		}

		return asset;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const
	{
		auto& search = m_AssetRegistry.find(handle);

		if (search == m_AssetRegistry.end())
			return s_NullMetadata;

		return search->second;
	}

	bool EditorAssetManager::SerialiseAssetRegistry()
	{
		auto assetDir = Project::GetAssetsDirectory();

		YAML::Emitter out;
		out << YAML::BeginMap << YAML::Key << "AssetRegistry" << YAML::Value;
		{
			out << YAML::BeginSeq;
			{
				for (const auto& [handle, metadata] : m_AssetRegistry)
				{
					auto relativePath = std::filesystem::relative(metadata.Filepath, assetDir);
					if (relativePath.empty())
					{
						Z_CORE_WARN("Registered asset '{}' is not within the project's asset directory: it will not be serialised with the registry", handle);
						continue;
					}

					out << YAML::BeginMap;
					{
						out << YAML::Key << "Handle" << YAML::Value << handle;
						out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeName(metadata.Type);
						out << YAML::Key << "Source" << YAML::Value << relativePath.string();
					}
					out << YAML::EndMap;
				}
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;

		std::filesystem::path registryFilepath = Project::GetAssetRegistryFilepath();

		std::ofstream fout(registryFilepath.c_str(), std::ios_base::out);
		fout << out.c_str();
	}

	bool EditorAssetManager::DeserialiseAssetRegistry()
	{
		std::filesystem::path registryFilepath = Project::GetAssetRegistryFilepath();

		if (!std::filesystem::exists(registryFilepath))
		{
			Z_CORE_WARN("Asset registry file not found at '{}'", registryFilepath.string().c_str());
			return false;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(registryFilepath.string());
		}
		catch (const YAML::ParserException& ex)
		{
			Z_CORE_ERROR("Failed to load asset registry file '{0}':\n{1}",
				registryFilepath.filename().string().c_str(), ex.what());
			return false;
		}

		auto assetNodes = data["AssetRegistry"];
		if (!assetNodes)
			return false;

		for (const auto& asset : assetNodes)
		{
			AssetHandle handle = asset["Handle"].as<uint64_t>();
			
			AssetMetadata metadata{};
			metadata.Type = Utils::AssetTypeFromName(asset["Type"].as<std::string_view>());
			metadata.Filepath = asset["Source"].as<std::string>();

			auto fullFilepath = Project::GetAssetsDirectory() / metadata.Filepath;
			if (!std::filesystem::exists(fullFilepath))
			{
				Z_CORE_WARN("Previously registered asset source file '{}' was not found: registry entry will be discarded", fullFilepath.string().c_str());
				continue;
			}

			m_AssetRegistry[handle] = metadata;
		}

		return true;
	}    
}
