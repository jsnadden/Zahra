#pragma once

#include "Zahra/Core/Ref.h"
#include "Zahra/Renderer/Shader.h"

namespace Zahra
{
	class Material : public RefCounted
	{
	public:
		virtual ~Material() {}

		static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name);
	};
}
