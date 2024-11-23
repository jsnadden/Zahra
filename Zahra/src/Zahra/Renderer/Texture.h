#pragma once

#include "Zahra/Core/Defines.h"

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

		virtual void SetData(void* data, uint32_t size) = 0;

		// this was used (in Renderer.cpp) for checking if a texture had been
		// bound already, but will likely not be needed anymore
		//virtual bool operator==(const Texture& other) const = 0;
	};

	enum class TextureFilterMode
	{
		Nearest,
		Linear
	};

	enum class TextureAddressMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	struct Texture2DSpecification
	{
		std::filesystem::path ImageFilepath;

		TextureFilterMode MinificationFilterMode = TextureFilterMode::Linear;
		TextureFilterMode MagnificationFilterMode = TextureFilterMode::Linear;
		TextureAddressMode AddressMode = TextureAddressMode::Repeat;
		
		// TODO: add mipmapping options
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const Texture2DSpecification& specification);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);

		virtual const Texture2DSpecification& GetSpecification() const = 0;

	};


}



