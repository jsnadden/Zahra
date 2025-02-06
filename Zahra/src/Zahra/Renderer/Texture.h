#pragma once

#include "Zahra/Assets/Asset.h"
#include "Zahra/Core/Buffer.h"
#include "Zahra/Core/Defines.h"
#include "Zahra/Renderer/Image.h"

#include <string>
#include <filesystem>

namespace Zahra
{
	typedef void* ImGuiTextureHandle;

	enum class TextureShape
	{
		Rectangular,
		Cubic,
		Volumetric
	};

	struct TextureSpecification
	{
		TextureShape Shape = TextureShape::Rectangular;
		ImageFormat Format = ImageFormat::SRGBA;
		uint32_t Width = 1, Height = 1;
		bool KeepLocalData = true;
		bool GenerateMips = false;
	};

	class Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		static AssetType GetAssetTypeStatic() { return AssetType::Texture2D; }
		virtual AssetType GetAssetType() const override { return GetAssetTypeStatic(); }
	};

	class Texture2D : public Texture
	{
	public:
		virtual const TextureSpecification& GetSpecification() const = 0;

		// TODO: find a way to avoid having to call this (callback queue in Image?)
		// NOTE: only use this method if the texture was created from an existing
		// image, and only do so after the image has already been resized
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		static Ref<Texture2D> CreateFromBuffer(const TextureSpecification& specification, Buffer imageData);
		static Ref<Texture2D> CreateFromImage2D(Ref<Image2D>& image);
		static Ref<Texture2D> CreateFlatColourTexture(const TextureSpecification& specification, uint32_t colour);		
	};

	class TextureImporter
	{
	public:
		static Ref<Texture2D> LoadTexture2DAsset(const AssetHandle& handle, const AssetMetadata& metadata);
		static Ref<Texture2D> LoadEditorIcon(const std::filesystem::path& sourceFilepath);
		static Buffer LoadImageData(const std::filesystem::path& sourceFilepath, uint32_t& widthOut, uint32_t& heightOut, ImageFormat& formatOut);
	};

}



