#pragma once
#include "Engine/Network/NetDefinition.hpp"
#include <stdint.h>
#include <vector>

const uint16_t INVALID_NETWORK_ID = 0xffff;

class NetConnection;
class NetObjectTypeDefinition;

class NetObject
{
public:
	NetObject(NetObjectTypeDefinition *defn);
	~NetObject();
public:
	uint16_t m_netID;
	size_t m_snapShotSize;
	NetObjectTypeDefinition* m_definition;
	void* m_localObj;
	std::vector<void*> m_lastSnapShot;
	void* m_currentSnapshot;
	float m_currHostTime;
};

class NetMessage;
class NetSession;

void NetObjectStopRelication(uint16_t net_id);
NetObject* NetObjectReplicate(void *object_ptr, uint8_t type_id);
void NetObjectSystemRegisterType(uint8_t id, NetObjectTypeDefinition& def);
void NetObjectStartup();
NetSession* GetNetObjectSession();
void RegisterNetObjectSession(NetSession* session);
void NetObjectCleanup();
void EstablishNetObjectMessages();
void NetObjectSystemStep();
void SetNetObjectRefreshRate(float hertz);
void SendNetObjectUpdateTo(NetConnection *cp);
void SendNetObjectUpdates();
uint16_t NetObjectGetUnusedID();
void SyncNetObjects(NetConnection* conn);
float GetHostTime();
void SetHostTime(float time);
float GetClientTime();
void SetClientTime(float time);
void ClearNetObjectList();