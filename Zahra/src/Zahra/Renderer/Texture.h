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



	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::filesystem::path& filepath); // TODO: make this (and the implementation) use std::filesystem::path instead
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);

	};


}



