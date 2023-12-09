#pragma once
#include <vector>
#include <string>

enum class ShaderDataType
{
	Float,
	Float2,
	Float3,
	Float4,
	Int,
	Int2,
	Int3,
	Int4,
	Mat3,
	Mat4,
	Bool,
};

struct BufferElement
{
	std::string name;
	ShaderDataType type;
	unsigned int size;
	unsigned int offset;
	bool normalized;

	unsigned int getShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:     return 4;
		case ShaderDataType::Float2:	return 4 * 2;
		case ShaderDataType::Float3:	return 4 * 3;
		case ShaderDataType::Float4:	return 4 * 4;
		case ShaderDataType::Int:		return 4;
		case ShaderDataType::Int2:		return 4 * 2;
		case ShaderDataType::Int3:		return 4 * 3;
		case ShaderDataType::Int4:		return 4 * 4;
		case ShaderDataType::Bool:		return 4;
		case ShaderDataType::Mat3:	    return 4 * 3 * 3;
		case ShaderDataType::Mat4:	    return 4 * 4 * 4;

		default: return 999;
		}
	}

	BufferElement(const std::string& name, ShaderDataType type, bool normalized = false)
		: name(name), type(type), size(getShaderDataTypeSize(type)), offset(0), normalized(normalized)
	{}

	unsigned int GetComponentCount() const
	{
		switch (type)
		{
		case ShaderDataType::Float:  return 1;
		case ShaderDataType::Float2: return 2;
		case ShaderDataType::Float3: return 3;
		case ShaderDataType::Float4: return 4;
		case ShaderDataType::Int:    return 1;
		case ShaderDataType::Int2:   return 2;
		case ShaderDataType::Int3:   return 3;
		case ShaderDataType::Int4:   return 4;
		case ShaderDataType::Mat3:   return 3 * 3;
		case ShaderDataType::Mat4:   return 4 * 4;
		case ShaderDataType::Bool:   return 4;
		}
		return 0;
	}
};

class BufferLayout
{
public:
	BufferLayout(const std::initializer_list<BufferElement>& elements)
		: bufferElements_(elements)
	{
		CalculateOffsetAndStride();
	}
	const std::vector<BufferElement>& GetElements() const { return bufferElements_; };
	
	unsigned int GetStride() { return stride_; }

	std::vector<BufferElement>::iterator begin() { return bufferElements_.begin(); }
	std::vector<BufferElement>::iterator end() { return bufferElements_.end(); }
	std::vector<BufferElement>::const_iterator begin() const { return bufferElements_.begin(); }
	std::vector<BufferElement>::const_iterator end() const { return bufferElements_.end(); }
private:
	void CalculateOffsetAndStride()
	{
		unsigned int offset = 0;
		stride_ = 0;
		for (auto& element : bufferElements_)
		{
			element.offset = offset;
			offset += element.size;
			stride_ += element.size;
		}
	}
private:
	std::vector<BufferElement> bufferElements_;
	unsigned int stride_;
};

class VertexBuffer
{
public:
	virtual ~VertexBuffer() {};
	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void SetLayout(const BufferLayout& layout) = 0; 
	virtual const BufferLayout& GetLayout() const = 0;
};

class IndexBuffer
{
public:
	virtual ~IndexBuffer() {};
	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};

class OpenGLVertexBuffer : public VertexBuffer
{
public:
	OpenGLVertexBuffer(float* vertices, unsigned int size);
	virtual ~OpenGLVertexBuffer();
	virtual void Bind() const;
	virtual void Unbind() const;
	virtual void SetLayout(const BufferLayout& layout);
	virtual const BufferLayout& GetLayout() const;
private:
	unsigned int rendererId_;
	float* vertices_;
	BufferLayout layout_;
};
 
class OpenGLIndexBuffer : public IndexBuffer
{
public:
	OpenGLIndexBuffer(unsigned int* indexes, unsigned int size);
	virtual ~OpenGLIndexBuffer();
	virtual void Bind();
	virtual void Unbind();
private:
	unsigned int rendererId_;
	unsigned int* indexes_;
};