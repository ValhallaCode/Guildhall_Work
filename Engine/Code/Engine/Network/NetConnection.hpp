#pragma once
#include "Engine/Network/NetAddress.hpp"
#include <stdint.h>

class NetMessage;
class NetSession;

class NetConnection
{
public:
	NetConnection();
	virtual ~NetConnection();

	virtual void Send(NetMessage* msg) = 0;
	virtual bool Receive(NetMessage** msg) = 0;

public:
	NetConnection* m_prev;
	NetConnection* m_next;
	NetSession* m_owner;
	net_address_t m_address;
	uint8_t m_connectionIndex; // LUID 
};