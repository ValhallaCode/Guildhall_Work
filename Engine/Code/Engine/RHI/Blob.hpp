#pragma once

typedef unsigned char byte_t;

// A blob [binary large buffer object] is a block of allocated memory
// and the size.  
class Blob
{
public:
	Blob(size_t initialSize = 0);
	~Blob(); // destroys buffer
	void AssignBuffer(void* buffer);
	void Resize(size_t size);

public:
	void* m_buffer;
	size_t m_bufferSize;
};
