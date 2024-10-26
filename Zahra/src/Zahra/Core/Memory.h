#pragma once

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <map>
#include <mutex>
#include <utility>


namespace Zahra
{
	
	struct AllocationStats
	{
		size_t TotalAllocated = 0;
		size_t TotalFreed = 0;
	};

	struct Allocation
	{
		void* Location = 0;
		size_t Size = 0;
		const char* Category = 0;
	};

	namespace Memory
	{
		const AllocationStats& GetAllocationStats();
	}

	template <class T>
	struct BaseAllocator
	{
		typedef T value_type;

		BaseAllocator() = default;
		template <class U>
		constexpr BaseAllocator(const BaseAllocator <U>&) noexcept {}

		T* allocate(std::size_t n)
		{
			if (n > (std::numeric_limits<std::size_t>::max)() / sizeof(T))
				throw std::bad_array_new_length();

			if (auto p = static_cast<T*>(std::malloc(n * sizeof(T))))
				return p;

			throw std::bad_alloc();
		}

		void deallocate(T* p, std::size_t n) noexcept
		{
			std::free(p);
		}

	};

	struct AllocatorData
	{
		using MapAllocator = BaseAllocator<std::pair<const void* const, Allocation>>;
		using StatsMapAllocator = BaseAllocator<std::pair<const char* const, AllocationStats>>;

		using AllocationStatsMap = std::map<const char*, AllocationStats, std::less<const char*>, StatsMapAllocator>;

		std::map<const void*, Allocation, std::less<const void*>, MapAllocator> m_AllocationMap;
		AllocationStatsMap m_AllocationStatsMap;

		std::mutex m_Mutex, m_StatsMutex;
	};


	class Allocator
	{
	public:
		static void Init();

		static void* AllocateRaw(size_t size);

		static void* Allocate(size_t size);
		static void* Allocate(size_t size, const char* category);
		static void* Allocate(size_t size, const char* file, int line);

		static void Free(void* location);

		static const AllocatorData::AllocationStatsMap& GetAllocationStatsMap() { return s_Data->m_AllocationStatsMap; }

	private:
		inline static AllocatorData* s_Data = nullptr;

	};

}

#if Z_TRACK_MEMORY
	#if defined(Z_PLATFORM_WINDOWS)

		_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
		void* __CRTDECL operator new(size_t size);

		_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
		void* __CRTDECL operator new[](size_t size);

		_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
		void* __CRTDECL operator new(size_t size, const char* category);

		_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
		void* __CRTDECL operator new[](size_t size, const char* category);

		_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
		void* __CRTDECL operator new(size_t size, const char* file, int line);

		_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
		void* __CRTDECL operator new[](size_t size, const char* file, int line);

		void __CRTDECL operator delete(void* location);

		void __CRTDECL operator delete(void* location, const char* category);

		void __CRTDECL operator delete(void* location, const char* file, int line);

		void __CRTDECL operator delete[](void* location);

		void __CRTDECL operator delete[](void* location, const char* category);

		void __CRTDECL operator delete[](void* location, const char* file, int line);

		#define znew new(__FILE__, __LINE__)
		#define zdelete delete

	#else
		#warning "Memory tracking not available on non-Windows platform"
		#define znew new
		#define zdelete delete
	#endif

#else
	#define znew new
	#define zdelete delete

#endif
