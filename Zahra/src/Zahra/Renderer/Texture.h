#pragma once

#include "Zahra/Assets/Asset.h"
#include "Zahra/Core/Defines.h"
#include "Zahra/Renderer/Image.h"

#include <string>
#include <filesystem>

namespace Zahra
{
	typedef void* ImGuiTextureHandle;

	class Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual const std::filesystem::path& GetFilepath() const = 0;

		static AssetType GetAssetTypeStatic() { return AssetType::Texture2D; }
		virtual AssetType GetAssetType() const override { return GetAssetTypeStatic(); }
	};

	struct Texture2DSpecification
	{
		ImageFormat Format = ImageFormat::SRGBA;
		uint32_t Width = 1, Height = 1;
		bool KeepLocalData = true;
		bool GenerateMips = false;
	};

	class Texture2D : public Texture
	{
	public:
		virtual const Texture2DSpecification& GetSpecification() const = 0;

		// TODO: find a way to avoid having to call this (callback queue in Image?)
		// NOTE: only use this method if the texture was created from an existing
		// image, and only do so after the image has already been resized
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		static Ref<Texture2D> CreateFromImage2D(Ref<Image2D>& image);
		static Ref<Texture2D> CreateFlatColourTexture(const Texture2DSpecification& specification, uint32_t colour);

		static Ref<Texture2D> LoadTexture2DFromSource(const AssetHandle& handle, const AssetMetadata& metadata);
	};

}



