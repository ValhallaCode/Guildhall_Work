#include "Engine/Input/BinaryStream.hpp"



eEndianness BinaryStream::GetHostOrder() const
{
	uint one = 0x01;
	byte_t* b = (byte_t*)one;
	if((uint)b == one)
		return LITTLE_ENDIAN;
	else
		return BIG_ENDIAN;

}

bool BinaryStream::IsBigEndian()
{
	return ((byte_t*)(&ENDIAN_CHECK))[0] == 0x01;
}


void BinaryStream::FlipBytes(void *buffer, uint const size)
{
	byte_t* bytes = (byte_t*)buffer;

	for (unsigned int index = 0; index < (size / 2); ++index)
	{
		unsigned int endIndex = (size - 1) - index;
		byte_t b1 = bytes[index];
		bytes[index] = bytes[endIndex];
		bytes[endIndex] = b1;
	}
	buffer = bytes;
}
