#pragma once

#include "Zahra/Assets/AssetManagerBase.h"
#include "Zahra/Projects/Project.h"

namespace Zahra
{
	class AssetManager
	{
	public:
		template <typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->GetAsset(handle).As<T>();
		}
	};
}
