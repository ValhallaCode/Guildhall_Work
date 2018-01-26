#include "Engine/Input/FileStream.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#pragma warning(push)
#pragma warning(disable: 4996)


FileBinaryStream::FileBinaryStream()
	: file_pointer(nullptr)
{}

FileBinaryStream::~FileBinaryStream()
{
	close();
}

bool FileBinaryStream::open_for_read(char const *filename)
{
	ASSERT_OR_DIE(!is_open(), "File Is Not Open");
	file_pointer = fopen(filename, "rb"); // open for read & binary
	return is_open();
}

bool FileBinaryStream::open_for_write(char const *filename)
{
	ASSERT_OR_DIE(!is_open(), "File Is Not Open");
	file_pointer = fopen(filename, "wb"); // open for write & binary
	return is_open();
}

void FileBinaryStream::close()
{
	if (is_open()) {
		fclose(file_pointer);
		file_pointer = nullptr;
	}
}

bool FileBinaryStream::read_byte(byte_t *out)
{
	return (read_bytes(out, 1) == 1);
}

bool FileBinaryStream::write_byte(byte_t const &value)
{
	return (write_bytes(&value, 1) == 1);
}

uint FileBinaryStream::read_bytes(void *out_buffer, uint const count)
{
	uint bytes_read = 0;
	if (is_open()) {
		bytes_read = (uint)fread(out_buffer, 1, count, file_pointer);
	}

	return bytes_read;
}

uint FileBinaryStream::write_bytes(void const* buffer, uint const size)
{
	uint bytes_read = 0;
	if (is_open()) {
		bytes_read = (uint)fwrite(buffer, 1, size, file_pointer);
	}

	return bytes_read;
}




#pragma warning(pop)