#pragma once

#include <memory>

namespace Zahra
{

	// TODO: finish replacing shared_ptr with a custom Ref implementation

	//class RefCounted
	//{
	//public:
	//	virtual ~RefCounted() = default;

	//	void IncrementRefCount() 
	//	{
	//		m_RefCount++;
	//	}

	//	void DecrementRefCount() 
	//	{
	//		m_RefCount--;
	//	}

	//	uint32_t GetRefCount() const { return m_RefCount; }

	//private:
	//	uint32_t m_RefCount = 0;
	//};

	//template <typename T>
	//class Ref
	//{
	//public:

	//	// default constructor
	//	Ref() : m_Raw(nullptr) {}

	//	// construct from nullptr
	//	Ref(std::nullptr_t null) : m_Raw(nullptr) {}

	//	// construct from non-null pointer
	//	Ref(T* ptr) : m_Raw(ptr)
	//	{
	//		static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted");

	//		IncrementRefCount();
	//	}

	//	// copy constructor
	//	template <typename T2>
	//	Ref(const Ref<T2>& other)
	//	{
	//		m_Raw = (T*)other.m_Raw;
	//		IncrementRefCount();
	//	}

	//	// move constructor
	//	template <typename T2>
	//	Ref(const Ref<T2>&& other)
	//	{
	//		m_Raw = (T*)other.m_Raw;
	//		other.m_Raw
	//	}

	//	


	//private:
	//	T* m_Raw;

	//	void IncrementRefCount()
	//	{
	//		if (m_Raw)
	//		{
	//			m_Raw->IncrementRefCount();
	//		}
	//	}

	//	void DecrementRefCount()
	//	{
	//		if (m_Raw)
	//		{
	//			m_Raw->DecrementRefCount();

	//			if (m_Raw->GetRefCount() == 0)
	//			{
	//				delete m_Raw;
	//				m_Raw = nullptr;
	//			}
	//		}
	//	}

	//	template <class T2>
	//	friend class Ref;

	//};

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}
