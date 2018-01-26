#include "Engine/Network/NetObject.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/Network/NetSession.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Network/NetConnection.hpp"
#include "Engine/Core/Interval.hpp"
#include "Engine/Core/Time.hpp"
#include <vector>

static float s_hostRefTime = 0.0f;
static float s_clientTime = 0.0f;

float GetHostTime()
{
	return s_hostRefTime;
}

void SetHostTime(float time)
{
	s_hostRefTime = time;
}

float GetClientTime()
{
	return s_clientTime;
}

void SetClientTime(float time)
{
	s_clientTime = time;
}

std::vector<NetObject*> s_netObjects;
std::vector<NetObjectTypeDefinition*> s_netObjectDefs;

uint16_t NetObjectGetUnusedID()
{
	for (uint16_t index = 0; index < s_netObjects.size(); ++index)
	{
		if (!s_netObjects[index])
			return index;
	}
	return INVALID_NETWORK_ID;
}

void ClearNetObjectList()
{
	s_netObjects.clear();
}

static NetSession* s_sessionRef = nullptr;

void RegisterNetObjectSession(NetSession* session)
{
	s_sessionRef = session;
}

NetSession* GetNetObjectSession()
{
	return s_sessionRef;
}

void NetObjectStartup()
{
	s_netObjects.resize(0xffff);
	s_netObjectDefs.resize(0xff);
}

void NetObjectCleanup()
{
	s_netObjectDefs.clear();
	s_netObjects.clear();
}

NetObjectTypeDefinition* NetObjectFindDefinition(uint8_t type_id)
{
	return s_netObjectDefs[type_id];
}

void NetObjectRegister(NetObject* obj)
{
	s_netObjects[obj->m_netID] = obj;
}

void NetObjectUnregister(NetObject* obj)
{
	s_netObjects[obj->m_netID] = nullptr;
}

NetObject* NetObjectFind(uint16_t net_id)
{
	return s_netObjects[net_id];
}

void NetObjectSystemRegisterType(uint8_t id, NetObjectTypeDefinition& def)
{
	s_netObjectDefs[id] = new NetObjectTypeDefinition(def);
	s_netObjectDefs[id]->m_typeID = id;
}

NetObject* NetObjectReplicate(void *object_ptr, uint8_t type_id)
{
	NetSession* session = GetNetObjectSession();
	if (!session->AmIHost())
		return nullptr;

	NetObjectTypeDefinition *defn = NetObjectFindDefinition(type_id);
	if (defn == nullptr) {
		return nullptr;
	}

	NetObject *nop = new NetObject(defn);

	nop->m_localObj = object_ptr;
	nop->m_netID = NetObjectGetUnusedID();

	if(defn->m_getSnapShotSize)
	{
		nop->m_snapShotSize = defn->m_getSnapShotSize();

		nop->m_currentSnapshot = ::calloc(1, nop->m_snapShotSize);
		nop->m_lastSnapShot.resize(session->m_maxConnectionCount + 1);
		nop->m_currHostTime = (float)GetCurrentTimeSeconds();
	}

	NetObjectRegister(nop);

	NetMessage create(NETOBJECT_CREATE_OBJECT);
	create.write_bytes(&type_id, sizeof(uint8_t));
	create.write_bytes(&nop->m_netID, sizeof(uint16_t));
	create.write_bytes(&nop->m_currHostTime, sizeof(float));

	defn->m_appendCreateInfo(&create, object_ptr);

	NetSession *sp = GetNetObjectSession();
	sp->SendMessageToOthers(create);

	return nop;
}

void SyncNetObjects(NetConnection* conn)
{
	for (NetObject* nop : s_netObjects)
	{
		if (!nop)
			continue;
		
		if (!nop->m_definition->m_appendSnapshot)
			continue;

		if(!nop->m_lastSnapShot[conn->m_connectionIndex])
			nop->m_lastSnapShot[conn->m_connectionIndex] = ::calloc(1, nop->m_snapShotSize);

		NetMessage create(NETOBJECT_CREATE_OBJECT);
		create.write_bytes(&nop->m_definition->m_typeID, sizeof(uint8_t));
		create.write_bytes(&nop->m_netID, sizeof(uint16_t));
		float current_time = (float)GetCurrentTimeSeconds();
		create.write_bytes(&current_time, sizeof(float));
		nop->m_definition->m_appendSnapshot(&create, nop->m_localObj);
		conn->Send(&create);
	}
}

void NetObjectStopRelication(uint16_t net_id)
{
	NetSession* session = GetNetObjectSession();
	if (!session->AmIHost())
		return;

	// remove from our system
	NetObject *nop = NetObjectFind(net_id);
	if (nop == nullptr) {
		return;
	}

	NetObjectUnregister(nop);

	// tell everyone 
	NetMessage msg(NETOBJECT_DESTROY_OBJECT);
	msg.write_bytes(&nop->m_netID, sizeof(uint16_t));

	// usually does nothing - no-op.
	if(nop->m_definition->m_appendDestroyInfo)
		nop->m_definition->m_appendDestroyInfo(&msg, nop->m_localObj);

	NetSession *sp = GetNetObjectSession();
	sp->SendMessageToOthers(msg);
}

void OnReceiveNetObjectCreate(NetMessage *msg)
{
	uint8_t type_id;
	msg->read_bytes(&type_id, sizeof(uint8_t));
	uint16_t net_id;
	msg->read_bytes(&net_id, sizeof(uint16_t));

	NetObjectTypeDefinition *defn = NetObjectFindDefinition(type_id);
	ASSERT_OR_DIE(defn != nullptr, "Net Object Not Defined!");

	NetObject *nop = new NetObject(defn);
	nop->m_netID = net_id;

	if(defn->m_getSnapShotSize)
	{
		nop->m_snapShotSize = defn->m_getSnapShotSize();
		nop->m_currentSnapshot = ::calloc(1, nop->m_snapShotSize);
		nop->m_lastSnapShot.push_back(::calloc(1, nop->m_snapShotSize));
	}
	else
	{
		nop->m_currentSnapshot = nullptr;
	}

	msg->read_bytes(&nop->m_currHostTime, sizeof(float));

	void *local_object = defn->m_processCreateInfo(msg, nop);
	ASSERT_OR_DIE(local_object != nullptr, "Local Object Not Defined!");
	nop->m_localObj = local_object;

	//defn->m_getCurrentSnapShot(nop->m_currentSnapshot, nop->m_localObj);
	if(defn->m_getCurrentSnapShot)
		defn->m_getCurrentSnapShot(nop->m_lastSnapShot.back(), nop->m_localObj);

	NetObjectRegister(nop); // register object with system
}

void NetObjectReceiveDestroy(NetMessage *msg)
{
	uint16_t net_id;
	msg->read_bytes(&net_id, sizeof(uint16_t));
	NetObject *nop = NetObjectFind(net_id);

	if (nop == nullptr) {
		return;
	}

	NetObjectUnregister(nop);

	// THIS is critical;
	nop->m_definition->m_processDestroyInfo(msg, nop->m_localObj);

	delete nop;
}

static Interval s_updateInterval;

void SetNetObjectRefreshRate(float hertz)
{
	s_updateInterval.SetFrequency(hertz);
}

void SendNetObjectUpdateTo(NetConnection *cp)
{
	for (NetObject *nop : s_netObjects)
	{
		if (!nop)
			continue;

		if (!nop->m_definition->m_appendSnapshot)
			continue;

		if (!nop->m_lastSnapShot[cp->m_connectionIndex])
			nop->m_lastSnapShot[cp->m_connectionIndex] = ::calloc(1, nop->m_snapShotSize);

		if (memcmp(nop->m_lastSnapShot[cp->m_connectionIndex], nop->m_currentSnapshot, nop->m_snapShotSize) == 0)
			continue;

		NetMessage update_msg(NET_OBJECT_UPDATE);
		update_msg.m_sender = GetNetObjectSession()->m_myConnection;
		update_msg.write_bytes(&nop->m_netID, sizeof(uint16_t));
		float current_time = (float)GetCurrentTimeSeconds();
		update_msg.write_bytes(&current_time, sizeof(float));
		nop->m_definition->m_appendSnapshot(&update_msg, nop->m_currentSnapshot);
		cp->Send(&update_msg);

		memcpy(nop->m_lastSnapShot[cp->m_connectionIndex], nop->m_currentSnapshot, nop->m_snapShotSize);
	}
}

void SendNetObjectUpdates()
{
	for (NetObject *nop : s_netObjects) 
	{
		if (!nop)
			continue;
		if(nop->m_definition->m_getCurrentSnapShot)
			nop->m_definition->m_getCurrentSnapShot(nop->m_currentSnapshot, nop->m_localObj);
	}


	NetSession* session = GetNetObjectSession();

	for(NetConnection* cp : session->m_connections)
	{
		if (!cp)
			continue;

		if (cp == session->m_myConnection)
			continue;

		SendNetObjectUpdateTo(cp);
	}

	//for (NetObject *nop : s_netObjects)
	//{
	//	if (!nop)
	//		continue;
	//
	//	memcpy(nop->m_lastSnapShot, nop->m_currentSnapshot, nop->m_snapShotSize);
	//}
}

void NetObjectSystemStep()
{
	NetSession* session = GetNetObjectSession();

	if (s_updateInterval.CheckAndReset()) {
		if (session->AmIHost()) {
			SendNetObjectUpdates();
		}
	}


	if (session->AmIClient())
	{
		for (NetObject *nop : s_netObjects)
		{
			if (!nop)
				continue;

			// Calculate Proper time to pass to apply
			float ref_initial_time = (nop->m_currHostTime - s_hostRefTime) + s_clientTime;
			float current_time_to_apply = (float)GetCurrentTimeSeconds() - ref_initial_time;

			if(nop->m_definition->m_applySnapshot)
				nop->m_definition->m_applySnapshot(nop->m_localObj, nop->m_lastSnapShot[0], current_time_to_apply);
		}
	}
}

void OnNetObjectUpdateRecieved(NetMessage *update_msg)
{
	uint16_t net_id;
	update_msg->read_bytes(&net_id, sizeof(uint16_t));

	NetObject *nop = NetObjectFind(net_id);
	if (nullptr == nop) {
		return;
	}

	update_msg->read_bytes(&nop->m_currHostTime, sizeof(float));

	// like current snapshot - we can just overwrite here
	// so only allocate this memory once. 
	nop->m_definition->m_processSnapshot(nop->m_lastSnapShot[0], update_msg);
}

void EstablishNetObjectMessages()
{
	NetSession* session = GetNetObjectSession();
	session->RegisterMessageDefinition(NETOBJECT_CREATE_OBJECT, OnReceiveNetObjectCreate);
	session->RegisterMessageDefinition(NETOBJECT_DESTROY_OBJECT, NetObjectReceiveDestroy);
	session->RegisterMessageDefinition(NET_OBJECT_UPDATE, OnNetObjectUpdateRecieved);
}

NetObject::NetObject(NetObjectTypeDefinition *defn)
	: m_definition(defn)
	, m_netID(INVALID_NETWORK_ID)
	, m_localObj(nullptr)
{

}

NetObject::~NetObject()
{
	for (uint index = 0; index < m_lastSnapShot.size(); ++index)
	{
		::free(m_lastSnapShot[index]);
	}
	m_lastSnapShot.clear();

	if(m_currentSnapshot)
		::free(m_currentSnapshot);
}