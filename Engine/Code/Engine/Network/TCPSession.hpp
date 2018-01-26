#pragma  once
#include "Engine/Network/NetSession.hpp"

class TCPSocket;

class TCPSession : public NetSession
{
public:
	TCPSession();
	virtual ~TCPSession();

public:
	// create a loopback - add it to connections, set my and host connection to it;
	virtual void Host(uint16_t port) override;

	// unimplemented so far;
	virtual bool Join(net_address_t const &addr) override; 

	// if I have a connection - destroy it.
	virtual void Leave() override;

	// Process all connections; 
	virtual void Update() override;

	// Process a specific message
	virtual void ProcessMessage(NetMessage* msg) override;

	void SendJoinInfo(NetConnection *cp);
	void OnJoinResponse(NetMessage *msg);
	bool StartListening();
	void StopListening();
	bool IsListening() const;
public:
	TCPSocket* m_listenSocket;
};