#include "Engine/Network/NetConnection.hpp"


NetConnection::NetConnection()
	:m_prev(nullptr)
	, m_next(nullptr)
	, m_owner(nullptr)
{

}

NetConnection::~NetConnection()
{

}

