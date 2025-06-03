#include "zpch.h"
#include "AssetLoader.h"

#include "Zahra/Renderer/Texture.h"

namespace Zahra
{
	using AssetLoadingFunction = std::function<Ref<Asset>(const AssetHandle&, const AssetMetadata&)>;
	static std::map<AssetType, AssetLoadingFunction> s_AssetLoadingFunctions = 
	{
		//{AssetType::Scene, },
		{ AssetType::Texture2D, TextureLoader::LoadTexture2DAsset },
		//{AssetType::Mesh, },
		//{AssetType::Material, },
		//{AssetType::Script, }
	};

	Ref<Asset> AssetLoader::LoadAssetFromSource(const AssetHandle& handle, const AssetMetadata& metadata)
	{
		auto& search = s_AssetLoadingFunctions.find(metadata.Type);
		if (search == s_AssetLoadingFunctions.end())
		{
			Z_CORE_ERROR("Unable to load assets of type {}: no loading function was found", Utils::AssetTypeName(metadata.Type));
			return nullptr;
		}

		return search->second(handle, metadata);
	}
}
