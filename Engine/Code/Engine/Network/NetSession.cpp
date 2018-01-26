#include "Engine/Network/NetSession.hpp"
#include "Engine/Network/NetConnection.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/Network/NetMessageDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

NetSession::NetSession()
	:m_myConnection(nullptr)
	, m_hostConnection(nullptr)
	, m_connectionList(nullptr)
	, m_maxConnectionCount(8)
{
	m_messageDefintions.resize(256);
}

NetSession::~NetSession()
{
	m_messageDefintions.clear();
}

bool NetSession::RegisterMessageDefinition(uint8_t msg_id, std::function<void(NetMessage*)> const &handler)
{
	if (!m_messageDefintions[msg_id])
	{
		NetMessageDefinition* defn = new NetMessageDefinition();
		defn->m_typeIndex = msg_id;
		defn->m_handler = handler;
		m_messageDefintions[msg_id] = defn;
		return true;
	}
	else
	{
		return false;
	}
}

NetMessageDefinition* NetSession::GetMessageDefinition(uint8_t id) const
{
	return m_messageDefintions[id];
}

uint8_t NetSession::GetFreeConnectionIndex() const 
{
	uint index = 0;
	for (index = 0; index < m_connections.size(); ++index) {
		if (m_connections[index] == nullptr) {
			return (uint8_t)index;
		}
	}

	if (index < m_maxConnectionCount) {
		return (uint8_t)index;
	}
	else {
		return INVALID_CONNECTION_INDEX;
	}

}

void NetSession::JoinConnection(uint8_t idx, NetConnection* conn)
{
	conn->m_connectionIndex = idx;

	std::string error_msg = "Could not Join Connection at: " + NetAddressToString(conn->m_address);
	ASSERT_OR_DIE((idx >= m_connections.size()) || (m_connections[idx] == nullptr), error_msg.c_str());

	if (idx >= m_connections.size()) {
		m_connections.resize(idx + 1);
	}

	m_connections[idx] = conn;
	AppendConnection(conn);
}

void NetSession::DestroyConnection(NetConnection *cp)
{
	if (nullptr == cp) {
		return;
	}

	if (m_myConnection == cp) {
		m_myConnection = nullptr;
	}

	if (m_hostConnection == cp) {
		m_hostConnection = nullptr;
	}

	if (cp->m_connectionIndex != INVALID_CONNECTION_INDEX) {
		m_connections[cp->m_connectionIndex] = nullptr;
		cp->m_connectionIndex = INVALID_CONNECTION_INDEX;
	}

	RemoveConnection(cp);

	delete cp;
	cp = nullptr;
}

NetConnection* NetSession::GetConnection(uint8_t idx)
{
	if (idx < m_connections.size()) {
		return m_connections[idx];
	}

	return nullptr;
}

void NetSession::SendMessageToOthers(NetMessage const &msg)
{
	NetConnection *cp = m_connectionList;
	while (cp != nullptr) {
		if (cp != m_myConnection) {
			cp->Send(new NetMessage(msg));
		}
		cp = cp->m_next;
	}
}

void NetSession::SetState(const eSessionState& new_state)
{
	m_state = new_state;
}

void NetSession::AppendConnection(NetConnection* conn)
{
	if (!m_connectionList)
	{
		m_connectionList = conn;
		return;
	}

	NetConnection* tracker = m_connectionList;
	while (tracker->m_next)
	{
		tracker = tracker->m_next;
	}

	tracker->m_next = conn;
	conn->m_prev = tracker;
}

void NetSession::RemoveConnection(NetConnection* conn)
{
	NetConnection* tracker = m_connectionList;
	if (!tracker)
		return;

	while (tracker)
	{
		if (tracker == conn)
		{
			if(tracker->m_next)
				tracker->m_next->m_prev = tracker->m_prev;

			if(tracker->m_prev)
				tracker->m_prev->m_next = tracker->m_next;
		}
		tracker = tracker->m_next;
	}

	if (conn == m_connectionList)
		m_connectionList = nullptr;

	conn = nullptr;
}

