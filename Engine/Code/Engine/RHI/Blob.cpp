#include "Engine//RHI/Blob.hpp"


Blob::Blob(size_t initialSize /*= 0*/)
{
	m_buffer = nullptr;
	m_bufferSize = initialSize;
}

Blob::~Blob()
{

}

void Blob::AssignBuffer(void* buffer)
{
	byte_t* bytes = (byte_t*)buffer;
	bytes;
	size_t newSize = sizeof(*bytes);
	Resize(newSize);

	m_buffer = buffer;
}

void Blob::Resize(size_t size)
{
	m_bufferSize = size;
}
