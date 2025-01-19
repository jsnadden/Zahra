#include "zpch.h"
#include "ProjectSerialiser.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

namespace Zahra
{
	ProjectSerialiser::ProjectSerialiser(Ref<Project> project)
		: m_Project(project) {}

	bool ProjectSerialiser::Serialise(const std::filesystem::path& filepath)
	{
		const auto& config = m_Project->GetConfig();

		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Project" << YAML::BeginMap;
			{
				out << YAML::Key << "Name" << YAML::Value << config.ProjectName;
				out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
				out << YAML::Key << "ScriptAssemblyFilepath" << YAML::Value << config.ScriptAssemblyFilepath.string();
				out << YAML::Key << "AutomateScriptEngineReloadAssembly" << YAML::Value << config.AutomateScriptEngineReloadAssembly;
				out << YAML::Key << "StartingSceneFilepath" << YAML::Value << config.StartingSceneFilepath.string();
			}
			out << YAML::EndMap;			
		}
		out << YAML::EndMap;

		std::ofstream fileWrite(filepath);
		fileWrite << out.c_str();
		fileWrite.close();

		return true;
	}

	bool ProjectSerialiser::Deserialise(const std::filesystem::path& filepath)
	{
		if (filepath.empty())
			return false;

		auto& config = m_Project->GetConfig();
		config.ProjectFilepath = filepath;
		config.ProjectDirectory = filepath.parent_path();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (const YAML::ParserException& ex)
		{
			Z_CORE_ERROR("Failed to deserialize project file '{0}'\n	{1}", filepath.string().c_str(), ex.what());
			return false;
		}

		if (auto& projectNode = data["Project"])
		{
			if (auto& name = projectNode["ProjectName"])
				config.ProjectName = name.as<std::string>();

			if (auto& assetDir = projectNode["AssetDirectory"])
				config.AssetDirectory = assetDir.as<std::string>();

			if (auto& scriptAssemblyPath = projectNode["ScriptAssemblyFilepath"])
				config.ScriptAssemblyFilepath = scriptAssemblyPath.as<std::string>();

			if (auto& autoReloadScripts = projectNode["AutomateScriptEngineReloadAssembly"])
				config.AutomateScriptEngineReloadAssembly = autoReloadScripts.as<bool>();

			if (auto& startingScene = projectNode["StartingSceneFilepath"])
				config.StartingSceneFilepath = startingScene.as<std::string>();

			return true;
		}

		return false;
	}
}
