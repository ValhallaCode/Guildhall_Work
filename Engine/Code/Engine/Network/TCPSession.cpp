#include "Engine/Network/TCPSession.hpp"
#include "Engine/Network/LoopBackConnection.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/Network/NetMessageDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Network/TCPConnection.hpp"

TCPSession::TCPSession()
	:m_listenSocket(nullptr)
{
	RegisterMessageDefinition((uint8_t)JOIN_RESPONSE, [=](NetMessage* msg) {this->OnJoinResponse(msg); });
}

TCPSession::~TCPSession()
{

}

void TCPSession::Host(uint16_t port)
{
	ASSERT_OR_DIE(!IsRunning(), "Connection is already Running!");

	m_myConnection = new LoopBackConnection();
	m_myConnection->m_address = GetMyAddress(port);

	JoinConnection(0, m_myConnection);
	m_hostConnection = m_myConnection;

	SetState(SESSION_CONNECTED);

	//return true;
}

bool TCPSession::Join(net_address_t const &addr)
{
	TCPConnection *host = new TCPConnection();
	host->m_address = addr;

	if (m_connections.size() == m_maxConnectionCount)
		return false;

	JoinConnection(0, host); // 0 for this class; 
	host->m_socket = new TCPSocket();
	host->m_socket->SetBlocking(false);

	// Try to connect to host
	if (!host->Connect(addr)) {
		Leave();
		return false;
	}

	m_hostConnection = host;
	m_myConnection = new LoopBackConnection();
	m_myConnection->m_address = GetMyAddress(addr.port);

	// 
	SetState(SESSION_CONNECTING);
	return true;
}

void TCPSession::Leave()
{
	DestroyConnection(m_myConnection);
	DestroyConnection(m_hostConnection);

	for (uint i = 0; i < m_connections.size(); ++i) {
		DestroyConnection(m_connections[i]);
	}

	StopListening();

	SetState(SESSION_DISCONNECTED);
}

void TCPSession::Update()
{
	if (IsListening()) {
		TCPSocket *socket = m_listenSocket->Accept();
		if (socket != nullptr) {
			TCPConnection *new_guy = new TCPConnection();
			new_guy->m_socket = socket;
			new_guy->m_socket->SetBlocking(false);
			new_guy->m_socket->EnableNagle(false);

			uint8_t conn_idx = GetFreeConnectionIndex();
			if (conn_idx == INVALID_CONNECTION_INDEX) {
				delete new_guy;
				new_guy = nullptr;
			}
			else {
				new_guy->m_address = socket->m_netAddr;
				JoinConnection(conn_idx, new_guy);
				SendJoinInfo(new_guy);
			}
		}
	}

	// Already Did It
	for (uint index = 0; index < m_connections.size(); ++index)
	{
		if (m_connections[index] == nullptr)
			continue;

		NetMessage* msg = nullptr;

		TCPConnection* tcp_conn = dynamic_cast<TCPConnection*>(m_connections[index]);
		if(tcp_conn)
			tcp_conn->m_socket->SetBlocking(false);

		while(m_connections[index]->Receive(&msg))
		{
			msg->m_sender = m_connections[index];
			ProcessMessage(msg);
			delete msg;
			msg = nullptr;

			if (!m_connections[index])
				break;
		} 
	}

	for (uint i = 0; i < m_connections.size(); ++i) {
		NetConnection *cp = m_connections[i];
		if ((cp != nullptr) && (cp != m_myConnection)) {
			TCPConnection *tcp_connection = (TCPConnection*)cp;
			if (tcp_connection->IsDisconnected()) {
				DestroyConnection(tcp_connection);
			}
		}
	}

	if (m_hostConnection == nullptr) {
		Leave();
	}
}

void TCPSession::ProcessMessage(NetMessage* msg)
{
	if (!msg)
		return;

	NetMessageDefinition* def = GetMessageDefinition(msg->m_messageTypeIndex);
	if(def)
		def->m_handler(msg);
}

void TCPSession::SendJoinInfo(NetConnection *cp)
{
	NetMessage *msg = new NetMessage(JOIN_RESPONSE);
	msg->write(cp->m_connectionIndex);

	cp->Send(msg);
}

void TCPSession::OnJoinResponse(NetMessage *msg)
{
	uint8_t my_conn_index;
	msg->read<uint8_t>(&my_conn_index);

	JoinConnection(my_conn_index, m_myConnection);
	SetState(SESSION_CONNECTED);
}

bool TCPSession::StartListening()
{
	if (!AmIHost()) {
		return false;
	}

	if (IsListening()) {
		return true;
	}

	m_listenSocket = new TCPSocket();
	if (m_listenSocket->Listen(m_myConnection->m_address.port)) {
		m_listenSocket->SetBlocking(false);
		return true;
	}
	else {
		delete m_listenSocket;
		m_listenSocket = nullptr;
		return false;
	}
}

void TCPSession::StopListening()
{
	if (IsListening()) {
		delete m_listenSocket;
		m_listenSocket = nullptr;
	}
}

bool TCPSession::IsListening() const
{
	return (nullptr != m_listenSocket);
}
