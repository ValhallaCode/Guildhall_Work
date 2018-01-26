#pragma once
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Network/NetAddress.hpp"
#include "Engine/Core/Logging.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Network/Net.hpp"

TCPSocket::TCPSocket()
	:m_socket((void*)INVALID_SOCKET)
	, m_isListening(false)
{
	
}

TCPSocket::~TCPSocket()
{
	Close();
}


void TCPSocket::EnableNagle(bool enabled)
{
	if (!IsValid()) {
		return;
	}

	int flag = enabled ? 1 : 0;
	SOCKET sock = (SOCKET)m_socket;
	int result = ::setsockopt(sock,
		IPPROTO_TCP,
		TCP_NODELAY,
		(char *)&flag,
		sizeof(int));

	if (result == SOCKET_ERROR) {
		DebuggerPrintf("Failed to disable Nagle's Algorithm. Error: %u", WSAGetLastError());
	}
}


bool TCPSocket::Join(const net_address_t& addr)
{
	if (IsValid())
	{
		return false;
	}

	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		return false;
	}

	sockaddr_storage internal_address;
	int addr_size = 0;
	SocketAddressFromNetAddress((sockaddr*)&internal_address, &addr_size, addr);

	int result = ::connect(sock, (sockaddr*)&internal_address, addr_size);
	if (result != 0)
	{
		::closesocket(sock);
		return false;
	}
	else
	{
		m_socket = (void*)sock;
		m_netAddr = addr;
		return true;
	}
}

void TCPSocket::Close()
{
	if (IsValid())
	{
		::closesocket((SOCKET)m_socket);
		m_socket = (void*)INVALID_SOCKET;
	}
}

bool TCPSocket::IsValid() const
{
	return (m_socket != (void*)INVALID_SOCKET);
}

bool TCPSocket::Listen(uint16_t port)
{
	if (IsValid())
	{
		return false;
	}

	std::vector<net_address_t> addresses = GetAddressFromHostName( "", port, true);
	if (addresses.empty())
	{
		return false;
	}

	SOCKET listen_socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == INVALID_SOCKET)
	{
		return false;
	}

	sockaddr_storage listen_address;
	uint addr_size = 0;
	SocketAddressFromNetAddress((sockaddr*)&listen_address, (int*)&addr_size, addresses[0]);

	int result = ::bind(listen_socket, (sockaddr*)&listen_address, (int)addr_size);
	if (result == SOCKET_ERROR)
	{
		::closesocket(listen_socket);
		return false;
	}

	// max line to be accepted
	int max_queued = 8;
	result = ::listen(listen_socket, (int)max_queued);
	if (result == SOCKET_ERROR)
	{
		::closesocket(listen_socket);
		return false;
	}

	m_isListening = true;
	m_socket = (void*)listen_socket;
	m_netAddr = addresses[0];
	return true;
}

bool TCPSocket::IsListening() const
{
	return m_isListening;
}

TCPSocket* TCPSocket::Accept()
{
	if (!IsListening())
	{
		return nullptr;
	}

	sockaddr_storage conn_addr;
	int con_addr_len = sizeof(conn_addr);

	SOCKET their_socket = ::accept((SOCKET)m_socket, (sockaddr*)&conn_addr, &con_addr_len);
	if (their_socket == SOCKET_ERROR)
	{
		::closesocket(their_socket);
		return nullptr;
	}

	net_address_t net_addr;
	if (!NetAddressFromSocketAddress(&net_addr, (sockaddr*)&conn_addr))
	{
		::closesocket(their_socket);
		return nullptr;
	}

	TCPSocket* their_tcp_socket = new TCPSocket();
	their_tcp_socket->m_socket = (void*)their_socket;
	their_tcp_socket->m_netAddr = net_addr;

	return their_tcp_socket;
}

void TCPSocket::SetBlocking(bool blocking)
{
	if (!IsValid())
	{
		return;
	}

	u_long non_blocking = blocking ? 0 : 1;
	::ioctlsocket((SOCKET)m_socket, FIONBIO, &non_blocking);
}

bool TCPSocket::CheckForDisconnect()
{
	if (!IsValid())
	{
		return true;
	}

	WSAPOLLFD file_desc;
	file_desc.fd = (SOCKET)m_socket;
	file_desc.events = POLLRDNORM;

	if (SOCKET_ERROR == WSAPoll(&file_desc, 1, 0))
	{
		return true;
	}

	if ((file_desc.revents & POLLHUP) != 0)
	{
		Close();
		return true;
	}

	return false;
}

uint TCPSocket::Send(const void* payload, uint size_bytes)
{
	if (!IsValid())
	{
		return 0;
	}

	if (IsListening())
	{
		// Recoverable error
		ASSERT_RECOVERABLE(false, "Socket attempting to Send is listening!");
		return 0;
	}

	if (payload == 0)
	{
		return 0;
	}

	int byte_sent = ::send((SOCKET)m_socket, (const char*)payload, (int)size_bytes, 0);

	if (byte_sent <= 0)
	{
		int error_val = WSAGetLastError();
		LogPrint("Socket failed with error: %u", error_val);
		Close();
		return 0;
	}

	ASSERT_OR_DIE((uint)byte_sent == size_bytes, "Bytes sent to socket do not match!");
	return byte_sent;
}

uint TCPSocket::Receive(void* payload, uint max_size)
{
	if (!IsValid() || max_size == 0)
	{
		return 0;
	}

	if (IsListening())
	{
		// Recoverable error
		ASSERT_RECOVERABLE(false, "Socket attempting to Send is listening!");
		return 0;
	}

	ASSERT_OR_DIE(payload != nullptr, "Payload was null!");

	int byte_read = ::recv((SOCKET)m_socket, (char*)payload, (int)max_size, 0);

	if (byte_read <= 0)
	{
		if (byte_read == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK)
			{
				return 0;
			}
			else {
				Close();
				return 0;
			}
		}
		else
		{
			CheckForDisconnect();
			return 0;
		}
	}
	else
	{
		return (uint)byte_read;
	}
}
