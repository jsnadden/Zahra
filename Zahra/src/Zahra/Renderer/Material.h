#pragma once

#include "Zahra/Assets/Asset.h"
#include "Zahra/Core/Ref.h"
#include "Zahra/Renderer/Shader.h"

namespace Zahra
{
	class Material : public Asset
	{
	public:
		virtual ~Material() {}

		static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name);

		static AssetType GetAssetTypeStatic() { return AssetType::Material; }
		virtual AssetType GetAssetType() const override { return GetAssetTypeStatic(); }
	};
}
