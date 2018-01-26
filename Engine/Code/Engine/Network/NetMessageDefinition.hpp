#pragma once

#include <stdint.h>
#include <functional>

class NetMessage;

class NetMessageDefinition
{
public:
	uint8_t m_typeIndex;
	std::function<void(NetMessage*)> m_handler;
};