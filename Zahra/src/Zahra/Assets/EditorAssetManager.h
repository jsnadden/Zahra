#pragma once

#include "Zahra/Core/Ref.h"
#include "Zahra/Assets/AssetManagerBase.h"

namespace Zahra
{
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;

	class EditorAssetManager : public AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) const override;
		//virtual const AssetMetadata& GetMetadata(AssetHandle handle) const override;
		const AssetMetadata& GetMetadata(AssetHandle handle) const;

		virtual bool IsAssetHandleValid(AssetHandle handle) const override { return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end(); }
		virtual bool IsAssetLoaded(AssetHandle handle) const override { return m_LoadedAssets.find(handle) != m_LoadedAssets.end(); }

		bool SerialiseAssetRegistry();
		bool DeserialiseAssetRegistry();

	private:
		AssetMap m_LoadedAssets;
		AssetRegistry m_AssetRegistry;
	};
}
