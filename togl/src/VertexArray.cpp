#include <glad/glad.h>

#include "VertexArray.h"
#include "Buffer.h"
#include <vector>
#include <memory>


static GLenum ShaderDataTypeTOOpenGLBaseType(ShaderDataType type)
{
	switch (type)
	{
	case ShaderDataType::Float: return GL_FLOAT;
	case ShaderDataType::Float2: return GL_FLOAT;
	case ShaderDataType::Float3: return GL_FLOAT;
	case ShaderDataType::Float4: return GL_FLOAT;
	case ShaderDataType::Int2: return GL_INT;
	case ShaderDataType::Int: return GL_INT;
	case ShaderDataType::Int3: return GL_INT;
	case ShaderDataType::Int4: return GL_INT;
	case ShaderDataType::Bool: return GL_INT;
	case ShaderDataType::Mat3: return GL_FLOAT;
	case ShaderDataType::Mat4: return GL_FLOAT;

	}
}

OpenGLVertexArray::OpenGLVertexArray()
{
	glCreateVertexArrays(1, &rendererId_);
}

void OpenGLVertexArray::Bind() const
{
	glBindVertexArray(rendererId_);
}

void OpenGLVertexArray::Unbind() const
{
	glBindVertexArray(0);
}

void OpenGLVertexArray::AddVertexBuffer(const VertexBuffer& vertexBuffer)
{
	glBindVertexArray(rendererId_);
	vertexBuffer.Bind();
	unsigned int index = 0;
	const auto& layout = vertexBuffer.GetLayout();
	for (const auto& element : layout)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index,
			element.GetComponentCount(),
			ShaderDataTypeTOOpenGLBaseType(element.type),
			element.normalized ? GL_TRUE : GL_FALSE,
			layout.GetStride(),
			(void*)element.offset);
		++index;
	}
	vertexBuffers_.push_back(vertexBuffer);
}

void OpenGLVertexArray::SetIndexBuffer(IndexBuffer& indexBuffer)
{
	glBindVertexArray(rendererId_);
	indexBuffer.Bind();

	indexBuffer_ = indexBuffer;
}
