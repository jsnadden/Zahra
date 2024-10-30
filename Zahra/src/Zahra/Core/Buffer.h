#pragma once

#include "Zahra/Core/Assert.h"
#include "Zahra/Core/Memory.h"
#include "Zahra/Core/Types.h"

namespace Zahra
{
	
	struct Buffer
	{
		void* Data = nullptr;
		uint64_t Size = 0;

		Buffer() = default;

		Buffer(const void* data, uint64_t size = 0)
			: Data((void*)data), Size(size)
		{}

		static Buffer Copy(const Buffer& other)
		{
			Buffer buffer;
			buffer.Allocate(other.Size);
			memcpy(buffer.Data, other.Data, other.Size);
			return buffer;
		}

		static Buffer Copy(const void* data, uint64_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(uint64_t size)
		{
			delete[] (byte*)Data;
			Data = nullptr;

			Size = size;
			if (size == 0) return;

			Data = znew byte[size];
		}

		void Release()
		{
			delete[] (byte*)Data;
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialise()
		{
			if (Data) memset(Data, 0, Size);
		}

		template <typename T>
		T& Read(uint64_t offset = 0)
		{
			return *(T*)((byte*)Data + offset);
		}

		template <typename T>
		const T& Read(uint64_t offset = 0) const
		{
			return *(T*)((byte*)Data + offset);
		}

		byte* ReadBytes(uint64_t size, uint64_t offset = 0) const
		{
			Z_CORE_ASSERT(offset + size <= Size, "Buffer overflow");

			byte* data = znew byte[size];
			memcpy(data, (byte*)Data + offset, size);

			return data;
		}

		void Write(const void* data, uint64_t size, uint64_t offset = 0)
		{
			Z_CORE_ASSERT(offset + size <= Size, "Buffer overflow");

			memcpy((byte*)Data + offset, data, size);
		}

		operator bool() const
		{
			return Data != nullptr;
		}

		byte& operator[](int index)
		{
			return ((byte*)Data)[index];
		}

		byte operator[](int index) const
		{
			return ((byte*)Data)[index];
		}

		template <typename T>
		T* As() const
		{
			return (T*)Data;
		}

		uint64_t GetSize() const
		{
			return Size;
		}

	};

}
