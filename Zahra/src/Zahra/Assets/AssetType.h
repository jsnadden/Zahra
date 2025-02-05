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
		static const char* AssetTypeName(AssetType type)
		{
			switch (type)
			{
				case AssetType::Scene:		return "Scene";
				case AssetType::Texture2D:	return "Texture2D";
				case AssetType::Mesh:		return "Mesh";
				case AssetType::Material:	return "Material";
				case AssetType::Script:		return "Script";
			}

			Z_CORE_ASSERT(false, "Invalid AssetType value");
			return "error";
		}

		static AssetType AssetTypeFromName(std::string_view typeName)
		{
			if (typeName == "Scene")		return AssetType::Scene;
			if (typeName == "Texture2D")	return AssetType::Texture2D;
			if (typeName == "Mesh")			return AssetType::Mesh;
			if (typeName == "Material")		return AssetType::Material;
			if (typeName == "Script")		return AssetType::Script;

			Z_CORE_ASSERT(false, "Unrecognised or invalid asset type name");
			return AssetType::None;
		}
	}
}
