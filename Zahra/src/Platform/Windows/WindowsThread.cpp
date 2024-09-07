#include "zpch.h"
#include "Zahra/Core/Thread.h"

namespace Zahra
{
	void Thread::SetNativeData(const std::string& name)
	{
		HANDLE threadHandle = m_Thread.native_handle();

		std::wstring wName(name.begin(), name.end());
		SetThreadDescription(threadHandle, wName.c_str());
		SetThreadAffinityMask(threadHandle, 8);
	}

}


