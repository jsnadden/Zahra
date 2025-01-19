#pragma once

namespace Zahra
{
	struct ProjectConfig
	{
		std::string ProjectName = "Untitled";
		std::filesystem::path ProjectDirectory;
		std::filesystem::path ProjectFilepath = ProjectDirectory / (ProjectName + ".zpj");

		std::filesystem::path AssetDirectory = "Assets";

		std::filesystem::path ScriptAssemblyFilepath = "Assets/Scripts/Binaries";
		bool AutomateScriptEngineReloadAssembly = false;

		// TODO: probably want an asset id here, rather than a filepath
		std::filesystem::path StartingSceneFilepath;
	};

	class Project : public RefCounted
	{
	public:
		Project() = default;
		~Project() = default;

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& projectFilepath);
		static void Save(const std::filesystem::path& projectFilepath);

		static ProjectConfig& GetConfig();
		static std::filesystem::path GetAssetsDirectory();

	private:
		ProjectConfig m_Config;

		// TODO: add an AssetManager here (the project should "own" its associated assets, in
		// the sense that unloading a project should clear them from memory)
	};
}
