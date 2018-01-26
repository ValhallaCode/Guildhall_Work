#pragma once
#include "Engine/Network/NetAddress.hpp"
#include <stdint.h>

class TCPSocket
{
public:
	void* m_socket;
	bool m_isListening;
	net_address_t m_netAddr;

public:
	TCPSocket();
	~TCPSocket();

	//CLIENT
	bool Join(const net_address_t& addr);
	void Close();
	bool IsValid() const;

	//HOST
	bool Listen(uint16_t port);
	bool IsListening() const;
	TCPSocket* Accept();

	//BOTH
	uint Send(const void* payload, uint size_bytes);
	uint Receive(void* payload, uint max_size);
	void SetBlocking(bool blocking);
	bool CheckForDisconnect();
	void EnableNagle(bool enabled);
};