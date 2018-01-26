#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <cstdint>

//winsock2 library
#pragma comment(lib, "ws2_32.lib")

typedef unsigned int uint;

bool NetSystemStartup();
void NetSystemShutdown();
bool EstablishUPNP(const std::string& map_name, uint16_t internal_port, uint16_t external_port, uint max_attempts = 10, uint sleep_interval = 100);
bool AddPortMapping(const std::string& port_map_name, const std::string& destination_ip, uint16_t external_port, uint16_t internal_port);
std::string GetControlAddress();
std::string GetExternalIP();