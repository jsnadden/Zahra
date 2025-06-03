#pragma once

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <map>
#include <mutex>
#include <utility>


namespace Zahra
{
	/**
	 * @brief Struct containing a summary of heap memory usage (either for the entire engine, or a subset of its allocations).
	 */
	struct AllocationStats
	{
		size_t TotalAllocated = 0; /**< @brief Total of allocated bytes since program start. */
		size_t TotalFreed = 0; /**< @brief Total of freed bytes since program start. */
	};

	/**
	 * @brief Struct containing the data of a single heap allocation.
	 */
	struct Allocation
	{
		void* Location = 0; /**< @brief Memory address. */
		size_t Size = 0; /**< @brief Allocation size, in bytes. */
		const char* Category = 0; /**< @brief Allocation category (typically based on the class whose method called new/malloc). */
	};

	namespace Memory
	{
		/**
		 * @brief Returns the AllocationStats struct tracking the entire engine's heap memory usage.
		 */
		const AllocationStats& GetAllocationStats();
	}

	/**
	 * @brief Templated custom allocator used internally by AllocatorData for setting up its own containers.
	 */
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

	/**
	 * @brief Struct containing engine allocation stats, and mutexes to maintain the statistics' thread-safety. Held internally in Allocator.
	 */
	struct AllocatorData
	{
		/**
		 * @brief Specialisation of BaseAllocator used specifically to allocate m_AllocationMap at initialisation.
		 */
		using MapAllocator = BaseAllocator<std::pair<const void* const, Allocation>>;

		/**
		 * @brief Specialisation of BaseAllocator used specifically to allocate m_AllocationStatsMap at initialisation.
		 */
		using StatsMapAllocator = BaseAllocator<std::pair<const char* const, AllocationStats>>;

		/**
		 * @brief A std::map containing allocation statistics.
		 */
		using AllocationStatsMap = std::map<const char*, AllocationStats, std::less<const char*>, StatsMapAllocator>;

		/**
		 * @brief A std::map containing the data of all engine allocations.
		 */
		std::map<const void*, Allocation, std::less<const void*>, MapAllocator> m_AllocationMap;

		/**
		 * @brief An AllocationStatsMap for all tracked allocation categories.
		 */
		AllocationStatsMap m_AllocationStatsMap;

		std::mutex m_Mutex, m_StatsMutex;
	};

	/**
	 * @brief The engine's core memory management system.
	 */
	class Allocator
	{
	public:
		/**
		 * @brief Initialises the memory manager, automatically called on first heap allocation.
		 */
		static void Init();

		/**
		 * @brief A primitive allocation call, for internal use during initialisation.
		 * @return Pointer to newly allocated heap memory.
		 */
		static void* AllocateRaw(size_t size);

		/**
		 * @brief A tracked, thread-safe allocation call, sans categorisation.
		 * 
		 * @return Pointer to newly allocated heap memory.
		 * 
		 * @param size Requested allocation size, in bytes.
		 */
		static void* Allocate(size_t size);

		/**
		 * @brief A tracked, thread-safe allocation call, categorised by custom label.
		 * 
		 * @return Pointer to newly allocated heap memory.
		 * 
		 * @param size Requested allocation size, in bytes.
		 * @param category Custom allocation label.
		 */
		static void* Allocate(size_t size, const char* category);

		/**
		 * @brief A tracked, thread-safe allocation call, categorised by location of call within source code.
		 * 
		 * @return Pointer to newly allocated heap memory.
		 * 
		 * @param size Requested allocation size, in bytes.
		 * @param file Filepath of source code calling for allocation.
		 * @param line Line number where call occurs in source code.
		 */
		static void* Allocate(size_t size, const char* file, int line);

		/**
		 * @brief A tracked, thread-safe memory deallocation call.
		 * 
		 * @param location Memory address of previously allocated data to be freed.
		 */
		static void Free(void* location);

		/**
		 * @brief Provides categorised heap allocation statistics.
		 * 
		 * @return Access to the internally-held AllocationStatsMap for all tracked allocation categories.
		 */
		static const AllocatorData::AllocationStatsMap& GetAllocationStatsMap() { return s_Data->m_AllocationStatsMap; }

	private:
		inline static AllocatorData* s_Data = nullptr;

	};

}

#if Z_TRACK_MEMORY
	// Override new/delete operators to go through the engine's internal
	// memory management system (platform dependent)
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
