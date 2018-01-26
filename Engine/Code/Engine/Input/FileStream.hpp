#pragma once
#include "Engine/Input/BinaryStream.hpp"
#include "Engine/Input/FileUtilities.hpp"

class FileBinaryStream : public BinaryStream
{
public:
	//----------------------------------------------------------------------------
	FileBinaryStream();

	//----------------------------------------------------------------------------
	~FileBinaryStream();

	//----------------------------------------------------------------------------
	bool open_for_read(char const *filename);

	//----------------------------------------------------------------------------
	bool open_for_write(char const *filename);

	//----------------------------------------------------------------------------
	void close();

	//----------------------------------------------------------------------------
	// BinaryStream Impl
	//----------------------------------------------------------------------------
	virtual bool read_byte(byte_t *out) override;

	//----------------------------------------------------------------------------
	virtual bool write_byte(byte_t const &value) override;

	//----------------------------------------------------------------------------
	// read/write 'count' bytes.  Returns number of bytes actually read. 
	// will return 0 on failure. 
	virtual uint read_bytes(void *out_buffer, uint const count);

	//----------------------------------------------------------------------------
	virtual uint write_bytes(void const* buffer, uint const size);

	//----------------------------------------------------------------------------
	inline bool is_open() const { return nullptr != file_pointer; }

public:
	FILE *file_pointer;   // file pointer;

};