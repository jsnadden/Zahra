#pragma once

namespace Zahra
{
	class ZGUID
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
	template <typename T> struct hash;

	template<>
	struct hash<Zahra::ZGUID>
	{
		std::size_t operator() (const Zahra::ZGUID& guid) const
		{
			return (uint64_t)guid;
		}
	};

}
