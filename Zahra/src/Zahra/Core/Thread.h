#pragma once

#include <string>
#include <thread>

namespace Zahra
{	
	class Thread
	{
	public:
		Thread(const std::string& name);

		template<typename Fn, typename... Args>
		void Dispatch(Fn&& func, Args&&... args)
		{
			m_Thread = std::thread(func, std::forward<Args>(args)...);
			SetName(m_Name);
		}

		void Join();
		std::thread::id GetID() const;
		void SetNativeData(const std::string& name);

	private:
		
		std::thread m_Thread;
		std::string m_Name;

	};

}

