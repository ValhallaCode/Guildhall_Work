#include "Engine/Network/NetMessage.hpp"




NetMessage::NetMessage()
	:m_payloadBytesUsed(0)
	, m_readBytes(0)
{

}

NetMessage::NetMessage(uint8_t type_index)
	:m_messageTypeIndex(type_index)
	, m_payloadBytesUsed(0)
	, m_readBytes(0)
{

}

bool NetMessage::read_byte(byte_t *out)
{
	return (read_bytes(out, 1) == 1);
}

bool NetMessage::write_byte(byte_t const &value)
{
	return (write_bytes(&value, 1) == 1);
}

uint NetMessage::write_bytes(void const *data, const uint size)
{
	uint bytes_read = size;
	if (size > 1024 - m_payloadBytesUsed) {
		bytes_read = 1024 - m_payloadBytesUsed;
	}

	void* new_data = const_cast<void*>(data);
	if (IsBigEndian())
		FlipBytes(new_data, bytes_read);

	std::memcpy(m_payload + m_payloadBytesUsed, new_data, bytes_read);
	m_payloadBytesUsed += bytes_read;
	return bytes_read;
}

uint NetMessage::read_bytes(void *out, const uint max_size)
{
	uint bytes_read = max_size;
	if (max_size > 1024 - m_readBytes) {
		bytes_read = 1024 - m_readBytes;
	}

	std::memcpy(out, m_payload + m_readBytes, bytes_read);

	if (IsBigEndian())
		FlipBytes(out, bytes_read);

	m_readBytes += bytes_read;
	return bytes_read;
}

void NetMessage::WriteString(const char* msg)
{
	std::string str_msg = msg;
	uint16_t new_size = (uint16_t)str_msg.size();
	if (new_size > 0xfffe) {
		new_size = 0xfffe;
	}

	byte_t* payload_start = m_payload + m_payloadBytesUsed;

	*(uint16_t*)payload_start = new_size;

	if (msg == nullptr)
		*(uint16_t*)payload_start = 0xffff;

	std::memcpy((void*)((m_payload + m_payloadBytesUsed) + 2), msg, new_size);
	m_payloadBytesUsed += new_size + 2;
}

std::string NetMessage::ReadString()
{
	byte_t* payload_start = m_payload + m_readBytes;
	uint16_t size_of_string = *(uint16_t*)payload_start;
	if (size_of_string == 0xffff)
	{
		return nullptr;
	}

	char msg[0xfffe];
	std::memcpy(msg, (void*)((m_payload + m_readBytes) + 2), size_of_string);
	m_readBytes += size_of_string + 2;

	// Fill Msg with null past payload info
	for (uint index = size_of_string; index < 0xfffe; ++index)
	{
		msg[index] = NULL;
	}

	std::string string = msg;
	return string;
}