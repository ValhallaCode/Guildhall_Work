#pragma once
#include "Engine/Network/NetConnection.hpp"
#include <queue>



class LoopBackConnection : public NetConnection
{
public:
	virtual ~LoopBackConnection();

	virtual void Send(NetMessage *msg);		// enqueue
	virtual bool Receive(NetMessage **msg);	// dequeue

public:
	std::queue<NetMessage*> m_messages;
};