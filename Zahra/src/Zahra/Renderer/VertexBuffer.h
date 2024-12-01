#pragma once

#include "Zahra/Core/Defines.h"
#include "Zahra/Renderer/ShaderTypes.h"

namespace Zahra
{
	struct VertexBufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		size_t Offset;
		bool Normalised;

		VertexBufferElement() 
			: Name(" "), Type(ShaderDataType::None), Size(0), Offset(0), Normalised(false)
		{}

		VertexBufferElement(ShaderDataType type, const std::string& name, bool normalised = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalised(normalised)
		{}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Bool:   return 1;

				case ShaderDataType::Int:    return 1;
				case ShaderDataType::Int2:   return 2;
				case ShaderDataType::Int3:   return 3;
				case ShaderDataType::Int4:   return 4;

				case ShaderDataType::Float:  return 1;
				case ShaderDataType::Float2: return 2;
				case ShaderDataType::Float3: return 3;
				case ShaderDataType::Float4: return 4;

				// TODO: should these be squared?
				case ShaderDataType::Mat2:   return 2 ;//* 2;
				case ShaderDataType::Mat3:   return 3 ;//* 3;
				case ShaderDataType::Mat4:   return 4 ;//* 4;
			}

			Z_CORE_ASSERT(false, "Invalid ShaderDataType");
			return 0;

		}

	};

	class VertexBufferLayout
	{
	public:

		VertexBufferLayout() {}

		VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		VertexBufferLayout(const std::vector<VertexBufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
		uint32_t GetElementCount() const { return (uint32_t)m_Elements.size(); }

		const uint32_t& GetStride() const { return m_Stride; }

		[[nodiscard]] std::vector<VertexBufferElement>::iterator begin() { return m_Elements.begin(); }
		[[nodiscard]] std::vector<VertexBufferElement>::iterator end() { return m_Elements.end(); }
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		std::vector<VertexBufferElement> m_Elements;
		uint32_t m_Stride = 0;

		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;

			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
			}

			m_Stride = offset;
		}

	};

	class VertexBuffer : public RefCounted
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void SetData(const void* data, uint64_t size) = 0;

		static Ref<VertexBuffer> Create(uint64_t size);
		static Ref<VertexBuffer> Create(const void* data, uint64_t size);
	};

}
