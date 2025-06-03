#pragma once

#include "Zahra/Assets/AssetType.h"
#include "Zahra/Core/UUID.h"

namespace Zahra
{
	using AssetHandle = UUID;
	using ImGuiTextureHandle = void*;

	struct AssetMetadata
	{
		AssetType Type = AssetType::None;
		std::filesystem::path Filepath;

		operator bool() const { return Type != AssetType::None; }
	};

	class Asset : public RefCounted
	{
	public:
		AssetHandle GetAssetHandle() const { return m_Handle; }

		virtual AssetType GetAssetType() const = 0;

	private:
		AssetHandle m_Handle;

	};
}
