#include "zpch.h"
#include "EditorAssetManager.h"

#include "Zahra/Assets/AssetLoader.h"

namespace Zahra
{
	static const AssetMetadata s_NullMetadata;

	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) const
	{
		const auto& metadata = GetMetadata(handle);
		if (!metadata) // i.e. if not found in registry
			return nullptr;

		Ref<Asset> asset;

		if (IsAssetLoaded(handle))
		{
			asset = m_LoadedAssets.at(handle);
		}
		else
		{
			asset = AssetLoader::LoadAssetFromSource(handle, metadata);

			if (!asset)
			{
				// TODO: handle failure to load
			}
		}

		return asset;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const
	{
		auto& search = m_AssetRegistry.find(handle);

		if (search == m_AssetRegistry.end())
			return s_NullMetadata;

		return search->second;
	}
    
}
