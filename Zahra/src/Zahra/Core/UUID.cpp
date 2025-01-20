#include "zpch.h"
#include "UUID.h"

#include <random>

namespace Zahra
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	// TODO: update the implementation when we add multithreading,
	// in order to maintain uniqueness across threads
	UUID::UUID() 
		: m_UUID(s_UniformDistribution(s_Engine)) {}

}

