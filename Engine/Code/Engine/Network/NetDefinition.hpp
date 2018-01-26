#pragma once
#include <stdint.h>

class NetMessage;
class NetObject;

typedef void(*AppendCreateInfoCB)(NetMessage*, void*);
typedef void(*AppendDestroyInfoCB)(NetMessage*, void*);
typedef void* (*ProcessCreateInfoCB)(NetMessage*, NetObject*);
typedef void(*ProcessDestroyInfoCB)(NetMessage*, void*);
typedef void(*ApplySnapShot)(void*, void*, float);
typedef void(*GetCurrentSnapShot)(void*, void*);
typedef void(*AppendSnapShot)(NetMessage*, void*);
typedef void(*ProcessSnapShot)(void*, NetMessage*);
typedef size_t(*SnapShotSize)();

class NetObjectTypeDefinition
{
public:
	uint8_t m_typeID;
	AppendCreateInfoCB m_appendCreateInfo;
	AppendDestroyInfoCB m_appendDestroyInfo;
	ProcessCreateInfoCB m_processCreateInfo;
	ProcessDestroyInfoCB m_processDestroyInfo;
	ApplySnapShot m_applySnapshot;
	GetCurrentSnapShot m_getCurrentSnapShot;
	AppendSnapShot m_appendSnapshot;
	ProcessSnapShot m_processSnapshot;
	SnapShotSize m_getSnapShotSize;
};