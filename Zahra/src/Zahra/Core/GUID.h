#pragma once

#include <xhash>

namespace Zahra
{
	class ZGUID  // TODO: use 128bit ids instead?
	{
	public:
		ZGUID();
		ZGUID(uint64_t guid)
			: m_GUID(guid) {}
		ZGUID(const ZGUID&) = default;

		operator uint64_t() const { return m_GUID; }

	private:
		uint64_t m_GUID;

	};
}

namespace std
{
	template<>
	struct hash<Zahra::ZGUID>
	{
		std::size_t operator() (const Zahra::ZGUID& guid) const // call operator, generates a hash for a given guid
		{
			return hash<uint64_t>()((uint64_t)guid);
		}
	};

}
