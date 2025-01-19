#pragma once

#include "Scene.h"

namespace Zahra
{

	class SceneSerialiser
	{
	public:
		SceneSerialiser(Ref<Scene> scene);

		void SerialiseYaml(const std::string& filepath);
		bool DeserialiseYaml(const std::string& filepath);

		void SeraliseBin(const std::string& filepath);
		bool DeseraliseBin(const std::string& filepath);

	private:
		Ref<Scene> m_Scene;

	};

}


