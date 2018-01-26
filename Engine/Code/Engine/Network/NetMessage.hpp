#pragma once
#include "Engine/Input/BinaryStream.hpp"
#include <string>

typedef unsigned int uint;
class NetConnection;

enum eCoreNetMessage : uint8_t
{
	JOIN_RESPONSE = 0,
	PING,
	PONG,
	RCS_COMMAND = 32,
	RCS_ECHO = 33,
	NETOBJECT_CREATE_OBJECT,
	NETOBJECT_DESTROY_OBJECT,
	NET_OBJECT_UPDATE,
	NUM_CORE_MESSAGES,
};

class NetMessage :  public BinaryStream 
{
public:
	NetMessage();
	NetMessage( uint8_t type_index );
	virtual bool read_byte(byte_t *out) override;
	virtual bool write_byte(byte_t const &value) override;
	virtual uint write_bytes( void const *data, const uint size );
	virtual uint read_bytes( void *out, const uint max_size );
	void WriteString(const char* msg);
	std::string ReadString();

public:
	uint8_t m_messageTypeIndex;
	NetConnection* m_sender;
	byte_t m_payload[1024];
	uint m_payloadBytesUsed;
	uint m_readBytes;
};