#include "zpch.h"
#include "AssetManagerBase.h"

namespace Zahra
{
	class AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) const = 0;
	};
}
