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
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(NULL);
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		Z_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "VertexBuffer has no layout")

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
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
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}

}
