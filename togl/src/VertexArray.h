#include "Buffer.h"

class VertexArray
{
public:
	virtual ~VertexArray() {}

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void AddVertexBuffer(const VertexBuffer& vertexBuffer) = 0;
	virtual void SetIndexBuffer(VertexBuffer& indexBuffer) = 0;
};

class OpenGLVertexArray : public VertexArray
{
public:
	OpenGLVertexArray();
	virtual ~OpenGLVertexArray() {}

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void AddVertexBuffer(const VertexBuffer& vertexBuffer);
	virtual void SetIndexBuffer(IndexBuffer& indexBuffer);
private:
	unsigned int rendererId_;
	std::vector<OpenGLVertexBuffer> vertexBuffers_;
	OpenGLIndexBuffer indexBuffer_;
};