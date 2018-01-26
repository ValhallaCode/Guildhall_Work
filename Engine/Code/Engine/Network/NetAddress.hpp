#pragma once
#include <vector>
#include <string>
#include <stdint.h>

typedef unsigned int uint;
struct sockaddr;

struct net_address_t
{
	// host name
	uint address;
	// port
	uint16_t port;
};

std::string NetAddressToString(net_address_t &addr);
std::string NetAddressToStringWithoutPort(net_address_t &addr);
net_address_t StringToNetAddress(const std::string& string);
bool NetAddressFromSocketAddress(net_address_t* out, sockaddr* socket_addr);
void SocketAddressFromNetAddress(sockaddr* out, int* out_size, const net_address_t& addr);
std::vector<net_address_t> GetAddressFromHostName(const char* host_name, uint16_t port, bool bindable);
net_address_t GetMyAddress(uint16_t port);
std::string GetRouterControlAddress();
bool EstablishUPNPConnection(const std::string& name, uint16_t internal_port, uint16_t external_port, uint max_attempts = 10, uint sleep_interval = 100);
std::string GetExternalIPAddress();