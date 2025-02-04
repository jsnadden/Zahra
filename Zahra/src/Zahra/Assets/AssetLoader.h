#pragma once

#include "Zahra/Assets/Asset.h"

namespace Zahra
{
	class AssetLoader
	{
	public:
		static Ref<Asset> LoadAssetFromSource(const AssetHandle& handle, const AssetMetadata& metadata);

	};
}
