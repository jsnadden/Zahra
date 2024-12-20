#pragma once

#include "Zahra/Core/Defines.h"
#include "Zahra/Renderer/Image.h"

#include <string>
#include <filesystem>

namespace Zahra
{
	class Texture : public RefCounted
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual const std::filesystem::path& GetFilepath() const = 0;

		//virtual void SetData(void* data, uint32_t size) = 0;
		//virtual void SetData(Ref<Image2D> srcImage) = 0;

		// this was used (in Renderer.cpp) for checking if a texture had been
		// bound already, but should be replaced with a Resource hash/GUID comparison
		//virtual bool operator==(const Texture& other) const = 0;
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

		bool KeepLocalData = false;
		
		// TODO: mipmapping, hdr etc.
	};

	class Texture2D : public Texture
	{
	public:
		virtual const Texture2DSpecification& GetSpecification() const = 0;

		// this does not resize the image itself, and should only be called after that has been done externally!!
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		static Ref<Texture2D> CreateFromFile(const Texture2DSpecification& specification, std::filesystem::path filepath);
		static Ref<Texture2D> CreateFromImage2D(const Ref<Image2D>& image);
		static Ref<Texture2D> CreateFlatColourTexture(const Texture2DSpecification& specification, uint32_t colour);
	};


}



