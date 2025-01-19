#pragma once

#include "Zahra/Projects/Project.h"

namespace Zahra
{
	class ProjectSerialiser
	{
	public:
		ProjectSerialiser(Ref<Project> project);

		bool Serialise(const std::filesystem::path& filepath);
		bool Deserialise(const std::filesystem::path& filepath);

	private:
		Ref<Project> m_Project;

	};
}

