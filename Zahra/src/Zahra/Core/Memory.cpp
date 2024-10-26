#include "zpch.h"
#include "Memory.h"

namespace Zahra
{
	static AllocationStats s_GlobalAllocationStats;

	static bool s_Initialising = false;

	const AllocationStats& Memory::GetAllocationStats()
	{
		return s_GlobalAllocationStats;
	}

	void Allocator::Init()
	{
		if (s_Data) return;

		s_Initialising = true;
		{
			AllocatorData* data = (AllocatorData*)AllocateRaw(sizeof(AllocatorData));
			new(data) AllocatorData();
			s_Data = data;
		}
		s_Initialising = false;
	}

	void* Allocator::AllocateRaw(size_t size)
	{
		return malloc(size);
	}

	void* Allocator::Allocate(size_t size)
	{
		// avoids 'new' making calls to s_Data before it's been initialised
		if (s_Initialising) return AllocateRaw(size);

		if (!s_Data) Init();

		void* location = malloc(size);

		{
			std::scoped_lock<std::mutex> lock(s_Data->m_Mutex);

			Allocation& alloc = s_Data->m_AllocationMap[location];
			alloc.Location = location;
			alloc.Size = size;

			s_GlobalAllocationStats.TotalAllocated += size;
		}

		return location;
	}

	void* Allocator::Allocate(size_t size, const char* category)
	{
		if (!s_Data) Init();

		void* location = malloc(size);

		{
			std::scoped_lock<std::mutex> lock(s_Data->m_Mutex);

			Allocation& alloc = s_Data->m_AllocationMap[location];
			alloc.Location = location;
			alloc.Size = size;
			alloc.Category = category;

			s_GlobalAllocationStats.TotalAllocated += size;
			if (category) s_Data->m_AllocationStatsMap[category].TotalAllocated += size;
		}

		return location;
	}

	void* Allocator::Allocate(size_t size, const char* file, int line)
	{
		if (!s_Data) Init();

		void* location = malloc(size);

		{
			std::scoped_lock<std::mutex> lock(s_Data->m_Mutex);

			Allocation& alloc = s_Data->m_AllocationMap[location];
			alloc.Location = location;
			alloc.Size = size;
			alloc.Category = file;

			s_GlobalAllocationStats.TotalAllocated += size;
			s_Data->m_AllocationStatsMap[file].TotalAllocated += size;
		}

		return location;
	}

	void Allocator::Free(void* location)
	{
		if (!location) return;

		bool found = false;

		{
			std::scoped_lock<std::mutex> lock(s_Data->m_Mutex);

			auto mapIterator = s_Data->m_AllocationMap.find(location);
			found = (mapIterator != s_Data->m_AllocationMap.end());

			if (found)
			{
				const Allocation& alloc = mapIterator->second;
				s_GlobalAllocationStats.TotalFreed += alloc.Size;
				if (alloc.Category) s_Data->m_AllocationStatsMap[alloc.Category].TotalFreed += alloc.Size;

				s_Data->m_AllocationMap.erase(location);
			}
		}

#ifndef Z_DIST
		if (!found) Z_CORE_WARN("Memory location {0} not found in allocation map", location);
#endif

		free(location);

	}

}

#if Z_TRACK_MEMORY && defined(Z_PLATFORM_WINDOWS)

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size)
{
	return Zahra::Allocator::Allocate(size);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new[](size_t size)
{
	return Zahra::Allocator::Allocate(size);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size, const char* category)
{
	return Zahra::Allocator::Allocate(size, category);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new[](size_t size, const char* category)
{
	return Zahra::Allocator::Allocate(size, category);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size, const char* file, int line)
{
	return Zahra::Allocator::Allocate(size, file, line);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new[](size_t size, const char* file, int line)
{
	return Zahra::Allocator::Allocate(size, file, line);
}

void __CRTDECL operator delete(void* location)
{
	return Zahra::Allocator::Free(location);
}

void __CRTDECL operator delete(void* location, const char* category)
{
	return Zahra::Allocator::Free(location);
}

void __CRTDECL operator delete(void* location, const char* file, int line)
{
	return Zahra::Allocator::Free(location);
}

void __CRTDECL operator delete[](void* location)
{
	return Zahra::Allocator::Free(location);
}

void __CRTDECL operator delete[](void* location, const char* category)
{
	return Zahra::Allocator::Free(location);
}

void __CRTDECL operator delete[](void* location, const char* file, int line)
{
	return Zahra::Allocator::Free(location);
}

#endif

