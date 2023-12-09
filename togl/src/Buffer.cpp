#include "Buffer.h"
#include "glad/glad.h"

OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, unsigned int size)
	:vertices_(vertices), layout_({})
{
	glGenBuffers(1, &rendererId_);
	Bind();
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}
OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
}

void OpenGLVertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, rendererId_);
}

void OpenGLVertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLVertexBuffer::SetLayout(const BufferLayout& layout)
{
	layout_ = layout;
}
const BufferLayout& OpenGLVertexBuffer::GetLayout() const
{
	return layout_;
}

OpenGLIndexBuffer::OpenGLIndexBuffer(unsigned int* indexes, unsigned int size)
	:indexes_(indexes)
{
	glGenBuffers(1, &rendererId_);
	Bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indexes, GL_STATIC_DRAW);
}
OpenGLIndexBuffer::~OpenGLIndexBuffer()
{

}
void OpenGLIndexBuffer::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId_);
}

void OpenGLIndexBuffer::Unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}