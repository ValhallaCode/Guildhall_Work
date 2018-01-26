#include "Engine/Network/TCPConnection.hpp"
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Network/NetMessage.hpp"

TCPConnection::TCPConnection()
	:m_lastIndex(0)
{
	
}

TCPConnection::~TCPConnection()
{

}

void TCPConnection::Send(NetMessage *msg)
{
	uint16_t package_size = (uint16_t)(msg->m_payloadBytesUsed + 1);
	m_socket->Send(&(package_size), 2);
	m_socket->Send(&msg->m_messageTypeIndex, 1);
	if(msg->m_payloadBytesUsed > 0)
		m_socket->Send(msg->m_payload, msg->m_payloadBytesUsed);
}

bool TCPConnection::Receive(NetMessage **msg)
{
	if(m_lastIndex < 2)
		m_lastIndex += m_socket->Receive(m_buffer + m_lastIndex, 2 - m_lastIndex);
	
	if (m_lastIndex < 2)
		return false;

	NetMessage byte_check;

	uint16_t bytes_to_read = *(uint16_t*)m_buffer;
	if (byte_check.IsBigEndian())
		byte_check.FlipBytes(&bytes_to_read, 2);

	uint16_t total_bytes = bytes_to_read + 2;

	if(m_lastIndex < total_bytes)
		m_lastIndex += m_socket->Receive(m_buffer + m_lastIndex, total_bytes - m_lastIndex);

	if (m_lastIndex < total_bytes)
		return false;

	if (byte_check.IsBigEndian())
		byte_check.FlipBytes(m_buffer, 1027);
	
	*msg = new NetMessage();
	NetMessage* ptr_to_msg = *msg;

	ptr_to_msg->m_payloadBytesUsed = bytes_to_read - 1;
	ptr_to_msg->m_messageTypeIndex = m_buffer[2];
	std::memcpy(ptr_to_msg->m_payload, m_buffer + 3, ptr_to_msg->m_payloadBytesUsed);
	m_lastIndex = 0;
	return true;
}

bool TCPConnection::Connect(const net_address_t& address)
{
	return m_socket->Join(address);
}

bool TCPConnection::IsDisconnected()
{
	if ((m_socket == nullptr) || (m_socket && !m_socket->IsValid())) {
		return true;
	}

	return false;
}
