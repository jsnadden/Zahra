#pragma once

#include <filesystem>

namespace Zahra
{
	struct ProjectConfig
	{
		std::string ProjectName;
		std::filesystem::path ProjectDirectory;
		std::filesystem::path ProjectFilepath;

		std::filesystem::path AssetDirectory;

		// TODO: ideally we would store asset ids here rather than filepaths
		// (asset manager should keep track of the source files internally)
		std::filesystem::path StartingSceneFilepath;
		std::filesystem::path ScriptAssemblyFilepath;

		bool AutoReloadScriptAssembly = true;
	};

	class Project : public RefCounted
	{
	public:
		Project() = default;
		~Project() = default;

		ProjectConfig& GetConfig() { return m_Config; }

		static Ref<Project> GetActive();

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& projectFilepath);
		static void Save(const std::filesystem::path& projectFilepath);

		static const std::string& GetProjectName();
		static const std::filesystem::path& GetProjectDirectory();
		static const std::filesystem::path& GetProjectFilepath();

		// TODO: add a helper method to assetManager which takes a path (relative to asset
		// directory), and returns the absolute path (use std::filesystem::canonical)

		static std::filesystem::path GetAssetsDirectory();
		static std::filesystem::path GetFontsDirectory();
		static std::filesystem::path GetMeshesDirectory();
		static std::filesystem::path GetScenesDirectory();
		static std::filesystem::path GetStartingSceneFilepath();
		static std::filesystem::path GetScriptsDirectory();
		static std::filesystem::path GetScriptAssemblyFilepath();
		static std::filesystem::path GetTexturesDirectory();

	private:
		ProjectConfig m_Config;

		// TODO: add an AssetManager here (the project should "own" its associated assets, in
		// the sense that unloading a project should clear them from memory)
	};
}
