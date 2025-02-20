#pragma once

#include "Zahra/Assets/Asset.h"
#include "Zahra/Renderer/IndexBuffer.h"
#include "Zahra/Renderer/VertexBuffer.h"

#include <glm/gtx/hash.hpp>

namespace Zahra
{
	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		//glm::vec3 Tangent1;
		//glm::vec3 Tangent2;
		glm::vec2 TextureCoordinates;

		bool operator==(const MeshVertex& other) const
		{
			return Position == other.Position
				&& Normal == other.Normal
				&& TextureCoordinates == other.TextureCoordinates;
		}
	};

	enum class MeshFileFormat
	{
		None,
		fbx,
		gltf, glb,
		obj,
		usd, usda, usdc, usdz
	};

	struct MeshSpecification
	{
		std::string Name = "anonymous_mesh";
		std::string SourceSubdirectory = "";
		MeshFileFormat SourceType = MeshFileFormat::None;
	};

	class Mesh : public Asset
	{
	public:
		virtual ~Mesh() {};

		virtual Ref<VertexBuffer> GetVertexBuffer() = 0;
		virtual Ref<IndexBuffer> GetIndexBuffer() = 0;

		static Ref<Mesh> CreateFromFile(MeshSpecification specification, const std::filesystem::path& filepath);

		static AssetType GetAssetTypeStatic() { return AssetType::Mesh; }
		virtual AssetType GetAssetType() const override { return GetAssetTypeStatic(); }
	};

}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<Zahra::MeshVertex>
	{
		std::size_t operator() (const Zahra::MeshVertex& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.Position) ^
				(hash<glm::vec3>()(vertex.Normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.TextureCoordinates) << 1);
		}
	};
}
