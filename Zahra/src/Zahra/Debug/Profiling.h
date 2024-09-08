#pragma once

#include "Zahra/Core/Scope.h"

#include <chrono>
#include <fstream>
#include <string>
#include <algorithm>
#include <thread>

// TODO: this profiler seems to have terrible performance, and skews its own results... FIND A BETTER WAY!!

namespace Zahra
{
	struct ProfileResult
	{
		const char* Name;
		long long  Start, End;
		uint32_t ThreadID;
	};


	struct InstrumentationSession
	{
		std::string Name;
	};


	class Instrumentor
	{
	public:
		Instrumentor()
			: m_CurrentSession(nullptr), m_ProfileCount(0)
		{}

		void BeginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			m_OutputStream.open(filepath);
			WriteHeader();
			m_CurrentSession = CreateScope<InstrumentationSession>();
			m_CurrentSession->Name = name;
		}

		void EndSession()
		{
			WriteFooter();
			m_OutputStream.close();
			m_CurrentSession.reset();
			m_ProfileCount = 0;
		}

		void WriteProfile(const ProfileResult& result)
		{
			if (m_ProfileCount++ > 0)
				m_OutputStream << ",";

			std::string name = result.Name;
			std::replace(name.begin(), name.end(), '"', '\'');

			m_OutputStream << "{";
			m_OutputStream << "\"cat\":\"function\",";
			m_OutputStream << "\"dur\":" << (result.End - result.Start) << ",";
			m_OutputStream << "\"name\":\"" << name << "\",";
			m_OutputStream << "\"ph\":\"X\",";
			m_OutputStream << "\"pid\":0,";
			m_OutputStream << "\"tid\":" << result.ThreadID << ",";
			m_OutputStream << "\"ts\":" << result.Start;
			m_OutputStream << "}";

			m_OutputStream.flush();
		}

		void WriteHeader()
		{
			m_OutputStream << "{\"otherData\": {}, \"traceEvents\":[";
			m_OutputStream.flush();
		}

		void WriteFooter()
		{
			m_OutputStream << "]}";
			m_OutputStream.flush();
		}

		 static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}

	private:
		Scope<InstrumentationSession> m_CurrentSession;
		std::ofstream m_OutputStream;
		int m_ProfileCount;

	};

	class InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name)
			: m_Name(name), m_Stopped(false)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}

		~InstrumentationTimer()
		{
			if (!m_Stopped)
			{
				Stop();
			}
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();

			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
			Instrumentor::Get().WriteProfile({ m_Name, start, end });

			m_Stopped = true;
		}

	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
		bool m_Stopped;

	};

}
//#define Z_PROFILING_ENABLED 1
#if Z_PROFILING_ENABLED
	// Resolve which function signature macro will be used. Note that this only
		// is resolved when the (pre)compiler starts, so the syntax highlighting
		// could mark the wrong one in your editor!
	#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	#define Z_FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
	#define Z_FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__FUNCSIG__)
	#define Z_FUNC_SIG __FUNCSIG__
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	#define Z_FUNC_SIG __FUNCTION__
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	#define Z_FUNC_SIG __FUNC__
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	#define Z_FUNC_SIG __func__
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
	#define Z_FUNC_SIG __func__
	#else
	#define Z_FUNC_SIG "Z_FUNC_SIG unknown!"
	#endif

	#define Z_PROFILE_BEGIN_SESSION(name, filepath) ::Zahra::Instrumentor::Get().BeginSession(name, filepath)
	#define Z_PROFILE_END_SESSION() ::Zahra::Instrumentor::Get().EndSession()
	#define Z_PROFILE_SCOPE(name) ::Zahra::InstrumentationTimer timer##__LINE__(name)
	#define Z_PROFILE_FUNCTION() Z_PROFILE_SCOPE(Z_FUNC_SIG)
#else
	#define Z_PROFILE_BEGIN_SESSION(name, filepath)
	#define Z_PROFILE_END_SESSION()
	#define Z_PROFILE_SCOPE(name)
	#define Z_PROFILE_FUNCTION()
#endif
