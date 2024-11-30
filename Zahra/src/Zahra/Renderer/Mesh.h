#pragma once

#include "Zahra/Renderer/IndexBuffer.h"
#include "Zahra/Renderer/VertexBuffer.h"

#include <glm/gtx/hash.hpp>

namespace Zahra
{
	struct MeshSpecification
	{
		std::filesystem::path Filepath;
	};

	// TODO: customise and expand this struct as we evolve our rendering architecture
	struct MeshVertex
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Colour = { 1.0f, 1.0f, 1.0f};
		glm::vec2 TextureCoordinates = { 0.0f, 0.0f };

		bool operator==(const MeshVertex& other) const {
			return Position == other.Position && Colour == other.Colour && TextureCoordinates == other.TextureCoordinates;
		}
	};

	class Mesh : public RefCounted
	{
	public:
		virtual ~Mesh() {};

		virtual Ref<VertexBuffer> GetVertexBuffer() = 0;
		virtual Ref<IndexBuffer> GetIndexBuffer() = 0;


		static Ref<Mesh> Create(MeshSpecification specification);

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
				(hash<glm::vec3>()(vertex.Colour) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.TextureCoordinates) << 1);
		}
	};

}
