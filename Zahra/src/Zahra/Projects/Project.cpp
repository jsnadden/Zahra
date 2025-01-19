#include "zpch.h"
#include "Project.h"

#include "ProjectSerialiser.h"

namespace Zahra
{
	static Ref<Project> s_ActiveProject;

	Ref<Project> Project::New()
	{
		s_ActiveProject = Ref<Project>::Create();
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& projectFilepath)
	{
		s_ActiveProject = Ref<Project>::Create();
		auto serialiser = ProjectSerialiser(s_ActiveProject);
		
		if (!serialiser.Deserialise(projectFilepath))
			s_ActiveProject = nullptr;
				
		return s_ActiveProject;
	}

	void Project::Save(const std::filesystem::path& projectFilepath)
	{
		auto serialiser = ProjectSerialiser(s_ActiveProject);
		serialiser.Serialise(projectFilepath);
	}

	ProjectConfig& Project::GetConfig()
	{
		Z_CORE_ASSERT(s_ActiveProject);
		return s_ActiveProject->m_Config;
	}

	std::filesystem::path Project::GetAssetsDirectory()
	{
		if (s_ActiveProject)
		{
			auto& root = s_ActiveProject->m_Config.ProjectDirectory;
			auto& sub = s_ActiveProject->m_Config.AssetDirectory;
			auto combined = root / sub;
			return combined;
		}
		
		return "";
	}
}
