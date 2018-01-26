#pragma once
#include "Engine/Network/NetConnection.hpp"
#include <vector>

class TCPSocket;

class TCPConnection : public NetConnection
{
public:
	TCPConnection();
	~TCPConnection();
	virtual void Send(NetMessage *msg) override;
	virtual bool Receive(NetMessage **msg) override;
	bool Connect(const net_address_t& address);
	bool IsDisconnected();

public:
	TCPSocket* m_socket;
	unsigned char m_buffer[1027];
	uint m_lastIndex;
};