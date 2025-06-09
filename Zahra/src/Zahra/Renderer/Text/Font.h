#pragma once

#include "Zahra/Assets/Asset.h"
#include "Zahra/Core/Scope.h"
#include "Zahra/Renderer/Texture.h"

#include <filesystem>

namespace Zahra
{
	struct MSDFData;

	struct FontAtlasSpecification
	{
		std::string Name;
		float FontSize;

		int Width, Height;
	};

	class Font : public Asset
	{
	public:
		enum class WritingSystem
		{
			Latin,
			Chinese,
			// TODO: add others to support localisation, making sure to add corresponding unicode ranges in Font.cpp
		};

		Font(const std::filesystem::path& filepath, WritingSystem writingSystem = WritingSystem::Latin);
		// TODO: another constructor to load from binary font data buffer
		// (using msdfgen::loadFontData instead of loadFont)

		~Font();

		Ref<Texture2D> GetAtlasTexture() { return m_AtlasTexture; }

		static AssetType GetAssetTypeStatic() { return AssetType::Font; }
		virtual AssetType GetAssetType() const override { return GetAssetTypeStatic(); }

	private:
		Ref<Texture2D> m_AtlasTexture;

		MSDFData* m_Data;
		WritingSystem m_WritingSystem = WritingSystem::Latin;
	};
}

