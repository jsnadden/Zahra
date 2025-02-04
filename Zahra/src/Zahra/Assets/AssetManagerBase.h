#pragma once

#include "Zahra/Assets/Asset.h"

namespace Zahra
{
	using AssetMap = std::map<AssetHandle, Ref<Asset>>;

	class AssetManagerBase : public RefCounted
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) const = 0;
		//virtual const AssetMetadata& GetMetadata(AssetHandle handle) const = 0;

		//virtual AssetHandle AddAsset(AssetType type) = 0;
		//virtual void RemoveAsset(AssetHandle handle) = 0;

		virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
	};
}
