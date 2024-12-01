#pragma once

#include "Zahra/Renderer/IndexBuffer.h"
#include "Zahra/Renderer/VertexBuffer.h"

#include <glm/gtx/hash.hpp>

namespace Zahra
{
	struct MeshVertex
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Tint = { 1.0f, 1.0f, 1.0f };
		glm::vec2 TextureCoordinates = { 0.0f, 0.0f };

		// TODO: customise and expand on this as I build on the 3d rendering architecture

		bool operator==(const MeshVertex& other) const {
			return Position == other.Position
				&& Tint == other.Tint
				&& TextureCoordinates == other.TextureCoordinates;
		}
	};

	struct MeshSpecification
	{
		std::filesystem::path Filepath;
	};

	class StaticMesh : public RefCounted
	{
	public:
		virtual ~StaticMesh() {};

		virtual Ref<VertexBuffer> GetVertexBuffer() = 0;
		virtual Ref<IndexBuffer> GetIndexBuffer() = 0;


		static Ref<StaticMesh> Create(MeshSpecification specification);

	private:


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
				(hash<glm::vec3>()(vertex.Tint) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.TextureCoordinates) << 1);
		}
	};

}
