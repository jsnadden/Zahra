#pragma once

#include "Zahra/Core/Memory.h"

#include <memory>
#include <atomic>

namespace Zahra
{

	class RefCounted
	{
	public:
		virtual ~RefCounted() = default;

		void IncrementRefCount() const
		{
			m_RefCount++;
		}

		void DecrementRefCount() const
		{
			m_RefCount--;
		}

		uint32_t GetRefCount() const { return m_RefCount.load(); }

	private:
		// using std::atomic ensures the thread safety
		// of incrementing/decrementing the ref count
		mutable std::atomic<uint32_t> m_RefCount = 0;
	};

	template<typename T>
	class Ref
	{
	public:
		// default constructor
		Ref()
			: m_Raw(nullptr)
		{
		}

		// construct from null pointer
		Ref(std::nullptr_t n)
			: m_Raw(nullptr)
		{
		}

		// construct from general pointer
		Ref(T* instance)
			: m_Raw(instance)
		{
			static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");

			Increment();
		}

		// copy constructor
		template<typename T2>
		Ref(const Ref<T2>& other)
		{
			m_Raw = (T*)other.m_Raw;
			Increment();
		}

		// move constructor
		template<typename T2>
		Ref(Ref<T2>&& other)
		{
			m_Raw = (T*)other.m_Raw;
			other.m_Raw = nullptr;
		}

		static Ref<T> CopyWithoutIncrement(const Ref<T>& other)
		{
			Ref<T> result = nullptr;
			result->m_Raw = other.m_Raw;
			return result;
		}

		~Ref()
		{
			Decrement();
		}

		// same-type copy constructor
		Ref(const Ref<T>& other)
			: m_Raw(other.m_Raw)
		{
			Increment();
		}

		// null pointer assignment
		Ref& operator=(std::nullptr_t)
		{
			Decrement();
			m_Raw = nullptr;
			return *this;
		}

		// same-type copy assignment
		Ref& operator=(const Ref<T>& other)
		{
			if (this == &other)
				return *this;

			other.Increment();
			Decrement();

			m_Raw = other.m_Raw;
			return *this;
		}

		// copy assignment
		template<typename T2>
		Ref& operator=(const Ref<T2>& other)
		{
			other.Increment();
			Decrement();

			m_Raw = other.m_Raw;
			return *this;
		}

		// move assignment
		template<typename T2>
		Ref& operator=(Ref<T2>&& other)
		{
			Decrement();

			m_Raw = other.m_Raw;
			other.m_Raw = nullptr;
			return *this;
		}

		// implicit bool casts
		operator bool() { return m_Raw != nullptr; }
		operator bool() const { return m_Raw != nullptr; }

		// member accessor
		T* operator->() { return m_Raw; }
		const T* operator->() const { return m_Raw; }

		// dereferencing
		T& operator*() { return *m_Raw; }
		const T& operator*() const { return *m_Raw; }

		// get raw pointer
		T* Raw() { return  m_Raw; }
		const T* Raw() const { return  m_Raw; }

		void Reset(T* instance = nullptr)
		{
			Decrement();
			m_Raw = instance;
		}

		template<typename T2>
		Ref<T2> As() const
		{
			return Ref<T2>(*this);
		}

		template<typename... Args>
		static Ref<T> Create(Args&&... args)
		{
#if Z_TRACK_MEMORY && defined(Z_PLATFORM_WINDOWS)
			return Ref<T>(new(typeid(T).name()) T(std::forward<Args>(args)...));
#else
			return Ref<T>(new T(std::forward<Args>(args)...));
#endif
		}

		bool operator==(const Ref<T>& other) const
		{
			return m_Raw == other.m_Raw;
		}

		bool operator!=(const Ref<T>& other) const
		{
			return !(*this == other);
		}

		bool EqualsObject(const Ref<T>& other)
		{
			if (!m_Raw || !other.m_Raw)
				return false;

			return *m_Raw == *other.m_Raw;
		}
	private:
		void Increment() const
		{
			if (m_Raw)
			{
				m_Raw->IncrementRefCount();
			}
		}

		void Decrement() const
		{
			if (m_Raw)
			{
				m_Raw->DecrementRefCount();

				if (m_Raw->GetRefCount() == 0)
				{
					delete m_Raw;
					m_Raw = nullptr;
				}
			}
		}

		template<class T2>
		friend class Ref;
		mutable T* m_Raw;
	};

	template<typename T>
	class WeakRef
	{
	public:
		WeakRef() = default;

		WeakRef(Ref<T> ref)
		{
			m_Raw = ref.Raw();
		}

		WeakRef(T* ptr)
		{
			m_Raw = ptr;
		}

		T* operator->() { return m_Raw; }
		const T* operator->() const { return m_Raw; }

		T& operator*() { return *m_Raw; }
		const T& operator*() const { return *m_Raw; }

		template<typename T2>
		WeakRef<T2> As() const
		{
			return WeakRef<T2>(dynamic_cast<T2*>(m_Raw));
		}

	private:
		T* m_Raw = nullptr;
	};

	/*template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}*/

}
