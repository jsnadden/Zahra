#pragma once

#include "Zahra/Core/Ref.h"
#include "Zahra/Assets/AssetManagerBase.h"

namespace Zahra
{
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;
	using ThumbnailMap = std::map<AssetHandle, ImGuiTextureHandle>;

	class EditorAssetManager : public AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) override;
		//virtual const AssetMetadata& GetMetadata(AssetHandle handle) const override;
		const AssetMetadata& GetMetadata(AssetHandle handle) const;

		const ImGuiTextureHandle GetThumbnailHandle(AssetHandle handle) const;

		virtual bool IsAssetHandleValid(AssetHandle handle) const override { return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end(); }
		virtual bool IsAssetLoaded(AssetHandle handle) const override { return m_LoadedAssets.find(handle) != m_LoadedAssets.end(); }

		bool SerialiseAssetRegistry();
		bool DeserialiseAssetRegistry();

	private:
		AssetMap m_LoadedAssets;
		AssetRegistry m_AssetRegistry;

		ThumbnailMap m_ThumbnailHandles;

		Ref<Asset> GetAssetIfLoaded(AssetHandle handle) const;

		void RegisterThumbnail(AssetHandle handle, const AssetMetadata& metadata, Ref<RefCounted> asset);
	};
}
