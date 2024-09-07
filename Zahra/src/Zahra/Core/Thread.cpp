#include "zpch.h"
#include "Thread.h"

namespace Zahra
{
	Thread::Thread(const std::string& name)
		: m_Name(name) {}

	void Thread::Join()
	{
		if (m_Thread.joinable()) m_Thread.join();
	}

	std::thread::id Thread::GetID() const
	{
		return m_Thread.get_id();
	}

}

