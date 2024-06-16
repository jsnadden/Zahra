#pragma once

namespace Zahra
{

	class Timestep
	{
	public:
		Timestep(float time = 0.0f) : m_Time(time) {}

		float GetSeconds() const { return m_Time; }
		float GetMilliSeconds() const { return 1000.0f * m_Time; }

	private:

		float m_Time;
	};

}