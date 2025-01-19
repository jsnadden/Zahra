#pragma once

#include "Zahra/Core/Defines.h"
#include "Zahra/Renderer/Image.h"

#include <string>
#include <filesystem>

namespace Zahra
{
	typedef void* ImGuiTextureHandle;

	class Texture : public RefCounted
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual const std::filesystem::path& GetFilepath() const = 0;

		// TODO: once we have a asset system in place this should be replaced with asset GUID
		virtual uint64_t GetHash() const = 0;

		//virtual void SetData(void* data, uint32_t size) = 0;
		//virtual void SetData(Ref<Image2D> srcImage) = 0;
	};

	enum class TextureFilterMode
	{
		Nearest,
		Linear
	};

	enum class TextureWrapMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	struct Texture2DSpecification
	{
		// TODO: not currently using these, as default values are set by VulkanImage
		// If we actually want to use them, they should be moved to Image.h anyway
		/*TextureFilterMode MinificationFilterMode = TextureFilterMode::Linear;
		TextureFilterMode MagnificationFilterMode = TextureFilterMode::Linear;
		TextureWrapMode WrapMode = TextureWrapMode::Repeat;*/

		ImageFormat Format = ImageFormat::SRGBA;

		bool KeepLocalData = true;
		
		// TODO: mipmapping, hdr etc.
	};

	class Texture2D : public Texture
	{
	public:
		virtual const Texture2DSpecification& GetSpecification() const = 0;

		// NOTE: only use this method if the texture was created from an existing
		// image, and only do so after the image has already been resized
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		static Ref<Texture2D> CreateFromFile(const Texture2DSpecification& specification, const std::filesystem::path& filepath);
		static Ref<Texture2D> CreateFromImage2D(Ref<Image2D>& image);
		static Ref<Texture2D> CreateFlatColourTexture(const Texture2DSpecification& specification, uint32_t colour);
	};


}



