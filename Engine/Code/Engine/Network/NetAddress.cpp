#pragma once
#include "Engine/Network/NetAddress.hpp"
#include "Engine/Network/Net.hpp"
#include <sstream>

std::string NetAddressToString(net_address_t &addr)
{
	unsigned char* char_ptr = (unsigned char*)&addr.address;
	unsigned char a = char_ptr[0];
	unsigned char b = char_ptr[1];
	unsigned char c = char_ptr[2];
	unsigned char d = char_ptr[3];

	return std::to_string(d) + "." + std::to_string(c) + "." + std::to_string(b) + "." + std::to_string(a) + ":" + std::to_string(addr.port);
}

std::string NetAddressToStringWithoutPort(net_address_t &addr)
{
	unsigned char* char_ptr = (unsigned char*)&addr.address;
	unsigned char a = char_ptr[0];
	unsigned char b = char_ptr[1];
	unsigned char c = char_ptr[2];
	unsigned char d = char_ptr[3];

	return std::to_string(d) + "." + std::to_string(c) + "." + std::to_string(b) + "." + std::to_string(a);
}

net_address_t StringToNetAddress(const std::string& string)
{
	std::istringstream iStringStream(string);
	std::string stringToken;
	std::vector<std::string> parts;
	while (std::getline(iStringStream, stringToken, '.'))
	{
		parts.push_back(stringToken);
	}

	int d = atoi(parts[0].c_str());
	int c = atoi(parts[1].c_str());
	int b = atoi(parts[2].c_str());

	std::istringstream iStringStream2(parts[3]);
	std::string stringToken2;
	std::vector<std::string> parts2;
	while (std::getline(iStringStream2, stringToken2, ':'))
	{
		parts2.push_back(stringToken2);
	}

	int a = atoi(parts2[0].c_str());

	unsigned char char_ptr[4];
	char_ptr[0] = *(unsigned char*)&a;
	char_ptr[1] = *(unsigned char*)&b;
	char_ptr[2] = *(unsigned char*)&c;
	char_ptr[3] = *(unsigned char*)&d;

	net_address_t addr;
	addr.port = (uint16_t)std::stoi(parts2[1]);
	addr.address = *(uint*)&char_ptr;
	return addr;
}

bool NetAddressFromSocketAddress(net_address_t* out, sockaddr* socket_addr)
{
	if (nullptr == socket_addr)
	{
		return false;
	}

	if (socket_addr->sa_family != AF_INET)
	{
		return false;
	}

	sockaddr_in* ipv4_addr = (sockaddr_in*)socket_addr;
	out->port = ntohs(ipv4_addr->sin_port);
	out->address = ntohl(ipv4_addr->sin_addr.S_un.S_addr);
	return true;
}

void SocketAddressFromNetAddress(sockaddr* out, int* out_size, const net_address_t& addr)
{
	sockaddr_in* out_addr = (sockaddr_in*)out;
	memset(out_addr, 0, sizeof(sockaddr_in));
	out_addr->sin_family = AF_INET;
	out_addr->sin_port = htons(addr.port);
	out_addr->sin_addr.S_un.S_addr = htonl(addr.address);

	*out_size = sizeof(sockaddr_in);
}

std::vector<net_address_t> GetAddressFromHostName(const char* host_name, uint16_t port, bool bindable)
{
	char service[16];
	sprintf_s(service, "%u", port);

	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // UDP = SOCK_DGRAM
	hints.ai_flags |= (bindable ? AI_PASSIVE : 0);

	addrinfo* results = nullptr;
	int status = ::getaddrinfo(host_name, service, &hints, &results);

	std::vector<net_address_t> addresses;
	if (status != 0)
	{
		return addresses;
	}

	addrinfo* addr = results;
	while (nullptr != addr)
	{
		net_address_t net_addr;
		if (NetAddressFromSocketAddress(&net_addr, addr->ai_addr))
		{
			addresses.push_back(net_addr);
		}

		addr = addr->ai_next;
	}

	::freeaddrinfo(results);
	return addresses;
}

net_address_t GetMyAddress(uint16_t port)
{
	std::vector<net_address_t> addresses = GetAddressFromHostName("", port, false);
	return addresses[0];
}

std::string GetRouterControlAddress()
{
	return GetControlAddress();
}

bool EstablishUPNPConnection(const std::string& name, uint16_t internal_port, uint16_t external_port, uint max_attempts /*= 10*/, uint sleep_interval /*= 100*/)
{
	return EstablishUPNP(name, internal_port, external_port, max_attempts, sleep_interval);
}

std::string GetExternalIPAddress()
{
	return GetExternalIP();
}