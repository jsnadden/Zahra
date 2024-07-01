#include "zpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Zahra
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Zahra::ShaderDataType::Bool:   return GL_BOOL;

			case Zahra::ShaderDataType::Int:    return GL_INT;
			case Zahra::ShaderDataType::Int2:   return GL_INT;
			case Zahra::ShaderDataType::Int3:   return GL_INT;
			case Zahra::ShaderDataType::Int4:   return GL_INT;

			case Zahra::ShaderDataType::Float:  return GL_FLOAT;
			case Zahra::ShaderDataType::Float2: return GL_FLOAT;
			case Zahra::ShaderDataType::Float3: return GL_FLOAT;
			case Zahra::ShaderDataType::Float4: return GL_FLOAT;

			case Zahra::ShaderDataType::Mat2:   return GL_FLOAT;
			case Zahra::ShaderDataType::Mat3:   return GL_FLOAT;
			case Zahra::ShaderDataType::Mat4:   return GL_FLOAT;
		}

		Z_CORE_ASSERT(false, "Invalid ShaderDataType");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		Z_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		Z_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		Z_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		Z_PROFILE_FUNCTION();

		glBindVertexArray(NULL);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		Z_PROFILE_FUNCTION();

		Z_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "VertexBuffer has no layout")

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			switch (element.Type)
			{
			case ShaderDataType::Bool:
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			{
				glEnableVertexAttribArray(index);
				glVertexAttribPointer(
					index,
					element.GetComponentCount(),
					ShaderDataTypeToOpenGLBaseType(element.Type),
					element.Normalised ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.Offset
				);
				index++;
				break;
			}
			case ShaderDataType::Mat2:
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			{
				uint8_t count = element.GetComponentCount();

				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(
						index,
						count,
						ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalised ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(sizeof(float) * count * i)
					);
					index++;
				}
				break;
			}
			default:
				Z_CORE_ASSERT(false, "Unknown ShaderDataType")
			}
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		Z_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}

}


