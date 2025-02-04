#pragma once

namespace Zahra
{
	enum class AssetType
	{
		None,
		Scene,
		Texture2D,
		Mesh,
		Material,
		Script,
	};

	namespace Utils
	{
		static const char* AssetTypeToString(AssetType type)
		{
			switch (type)
			{
				case Zahra::AssetType::Scene:		return "Scene";
				case Zahra::AssetType::Texture2D:	return "Texture2D";
				case Zahra::AssetType::Mesh:		return "Mesh";
				case Zahra::AssetType::Material:	return "Material";
				case Zahra::AssetType::Script:		return "Script";
			}

			Z_CORE_ASSERT(false, "Invalid AssetType value");
			return "error";
		}
	}
}
