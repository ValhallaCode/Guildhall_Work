#include "Engine/Network/LoopBackConnection.hpp"



LoopBackConnection::~LoopBackConnection()
{
	
}

void LoopBackConnection::Send(NetMessage *msg)
{
	m_messages.push(msg);
}

bool LoopBackConnection::Receive(NetMessage **msg)
{
	if (m_messages.empty())
		return false;

	*msg = m_messages.front();
	m_messages.pop();
	return (*msg == nullptr) ? false : true;
}
