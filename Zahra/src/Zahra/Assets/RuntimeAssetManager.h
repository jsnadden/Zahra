#pragma once

#include "Zahra/Assets/AssetManagerBase.h"

namespace Zahra
{
	class RuntimeAssetManager : public AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) const override;
		virtual const AssetMetadata& GetMetadata(AssetHandle handle) const override;

		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
	};
}
