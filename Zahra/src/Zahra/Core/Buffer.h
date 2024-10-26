#pragma once

#include "Zahra/Core/Assert.h"
#include "Zahra/Core/Types.h"

namespace Zahra
{
	
	struct Buffer
	{
		void* Data = nullptr;
		uint64_t Size = 0;

		Buffer() = default;

		Buffer(const void* data, uint64_t size = 0)
			: Data((void*)data), Size(size)
		{}

		void Allocate(uint64_t size)
		{
			
		}

	};

}
