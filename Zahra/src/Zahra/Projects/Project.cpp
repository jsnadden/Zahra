#include "zpch.h"
#include "Project.h"

#include "ProjectSerialiser.h"

namespace Zahra
{
	static Ref<Project> s_ActiveProject;

	Ref<Project> Project::GetActive()
	{
		return s_ActiveProject;
	}

	Ref<Project> Project::New()
	{
		s_ActiveProject = Ref<Project>::Create();
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& projectFilepath)
	{
		auto project = Ref<Project>::Create();
		
		auto serialiser = ProjectSerialiser(project);
		if (serialiser.Deserialise(projectFilepath))
		{
			s_ActiveProject = project;
			return s_ActiveProject;
		}
				
		return nullptr;
	}

	void Project::Save(const std::filesystem::path& projectFilepath)
	{
		auto serialiser = ProjectSerialiser(s_ActiveProject);
		serialiser.Serialise(projectFilepath);
	}

	const std::string& Project::GetProjectName()
	{
		Z_CORE_ASSERT(s_ActiveProject);
		return s_ActiveProject->m_Config.ProjectName;
	}

	const std::filesystem::path& Project::GetProjectDirectory()
	{
		Z_CORE_ASSERT(s_ActiveProject);
		return s_ActiveProject->m_Config.ProjectDirectory;
	}

	const std::filesystem::path& Project::GetProjectFilepath()
	{
		Z_CORE_ASSERT(s_ActiveProject);
		return s_ActiveProject->m_Config.ProjectFilepath;
	}

	std::filesystem::path Project::GetAssetsDirectory()
	{
		if (s_ActiveProject && !s_ActiveProject->m_Config.AssetDirectory.empty())
		{
			return s_ActiveProject->m_Config.ProjectDirectory
				/ s_ActiveProject->m_Config.AssetDirectory;
		}
		
		return std::filesystem::path();
	}
	std::filesystem::path Project::GetFontsDirectory()
	{
		if (s_ActiveProject && !s_ActiveProject->m_Config.AssetDirectory.empty())
		{
			return s_ActiveProject->m_Config.ProjectDirectory
				/ s_ActiveProject->m_Config.AssetDirectory
				/ "Fonts";
		}

		return std::filesystem::path();
	}

	std::filesystem::path Project::GetMeshesDirectory()
	{
		if (s_ActiveProject && !s_ActiveProject->m_Config.AssetDirectory.empty())
		{
			return s_ActiveProject->m_Config.ProjectDirectory
				/ s_ActiveProject->m_Config.AssetDirectory
				/ "Meshes";
		}

		return std::filesystem::path();
	}

	std::filesystem::path Project::GetScenesDirectory()
	{
		if (s_ActiveProject && !s_ActiveProject->m_Config.AssetDirectory.empty())
		{
			return s_ActiveProject->m_Config.ProjectDirectory
				/ s_ActiveProject->m_Config.AssetDirectory
				/ "Scenes";
		}

		return std::filesystem::path();
	}

	std::filesystem::path Project::GetStartingSceneFilepath()
	{
		if (s_ActiveProject && !s_ActiveProject->m_Config.StartingSceneFilepath.empty())
		{
			return s_ActiveProject->m_Config.ProjectDirectory
				/ s_ActiveProject->m_Config.StartingSceneFilepath;
		}

		return std::filesystem::path();
	}

	std::filesystem::path Project::GetScriptsDirectory()
	{
		if (s_ActiveProject && !s_ActiveProject->m_Config.AssetDirectory.empty())
		{
			return s_ActiveProject->m_Config.ProjectDirectory
				/ s_ActiveProject->m_Config.AssetDirectory
				/ "Scripts";
		}

		return std::filesystem::path();
	}

	std::filesystem::path Project::GetScriptAssemblyFilepath()
	{
		if (s_ActiveProject && !s_ActiveProject->m_Config.ScriptAssemblyFilepath.empty())
		{
			return s_ActiveProject->m_Config.ProjectDirectory
				/ s_ActiveProject->m_Config.ScriptAssemblyFilepath;
		}

		return std::filesystem::path();
	}

	std::filesystem::path Project::GetTexturesDirectory()
	{
		if (s_ActiveProject && !s_ActiveProject->m_Config.AssetDirectory.empty())
		{
			return s_ActiveProject->m_Config.ProjectDirectory
				/ s_ActiveProject->m_Config.AssetDirectory
				/ "Textures";
		}

		return std::filesystem::path();
	}
}
