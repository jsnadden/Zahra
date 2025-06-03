#pragma once

#include "stdint.h"

namespace Zahra
{
	/**
	 * @brief A 64-bit universally-unique identifier implementation.
	 * 
	 * Primarily used by the engine's ECS systems (e.g. in Scene and ScriptEngine), especially within the Meadow editor app.
	 */
	class UUID
	{
	public:
		/**
		 * @brief Default constructor, generating pseudorandom id value.
		 */
		UUID();

		/**
		 * @brief Construct with assigned id value.
		 * @param uuid The value to be used as the underlying id value.
		 */
		UUID(uint64_t uuid)
			: m_UUID(uuid) {}

		/**
		 * @brief Default copy constructor.
		 */
		UUID(const UUID&) = default;

		/**
		 * @brief Cast to underlying id value.
		 */
		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;

	};
}

namespace std
{
	template <typename T> struct hash;

	/**
	 * @brief Custom std::hash implementation for UUID
	 */
	template<>
	struct hash<Zahra::UUID>
	{
		std::size_t operator() (const Zahra::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

}
