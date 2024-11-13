#pragma once

#include "Zahra/Renderer/Shader.h"

namespace Zahra
{
	class VulkanShaderResourceManager
	{
	public:


	private:
		using SetIndex = uint32_t;
		using BindingIndex = uint32_t;
		template <typename T> using IndexedMap = std::map<SetIndex, std::map<BindingIndex, T>>;

		std::vector<IndexedMap<ShaderResource>> m_Resources;

	};

}
