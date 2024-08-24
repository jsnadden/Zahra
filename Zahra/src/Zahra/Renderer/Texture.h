#pragma once

#include "Zahra/Core/Base.h"

#include <string>

namespace Zahra
{
	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual const std::filesystem::path& GetFilepath() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};



	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path); // TODO: make this (and the implementation) use std::filesystem::path instead
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);

	};


}



