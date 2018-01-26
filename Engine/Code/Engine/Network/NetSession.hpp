#pragma once
#include "Engine/Network/NetAddress.hpp"
#include <stdint.h>
#include <vector>
#include <functional>

class NetMessageDefinition;
class NetConnection;
class NetMessage;

enum eSessionState
{
	SESSION_DISCONNECTED,
	SESSION_CONNECTING,
	SESSION_CONNECTED,
};

constexpr uint8_t INVALID_CONNECTION_INDEX = 0xff;

class NetSession
{
public:
	NetSession();
	virtual ~NetSession();

public:
	virtual void Host(uint16_t port) = 0;
	virtual bool Join(net_address_t const &addr) = 0; // production version: session_join_info_t instead
	virtual void Leave() = 0;
	virtual void Update() = 0;
	virtual void ProcessMessage(NetMessage* msg) = 0;

public:
	bool RegisterMessageDefinition(uint8_t msg_id, std::function<void(NetMessage*)> const &handler);
	NetMessageDefinition* GetMessageDefinition(uint8_t id) const;
	inline bool AmIHost() const { return (m_myConnection == m_hostConnection) && (m_hostConnection != nullptr); };
	inline bool AmIClient() const { return (m_myConnection != m_hostConnection) && (m_myConnection != nullptr); };
	inline bool IsRunning() const { return (m_myConnection != nullptr); };
	inline bool IsReady() const { return (m_state == SESSION_CONNECTED); };
	uint8_t GetFreeConnectionIndex() const;
	void JoinConnection(uint8_t idx, NetConnection* conn);
	void DestroyConnection(NetConnection *cp);
	NetConnection* GetConnection(uint8_t idx);
	void SendMessageToOthers(NetMessage const &msg);
	void SetState(const eSessionState& new_state);
	void AppendConnection(NetConnection* conn);
	void RemoveConnection(NetConnection* conn);

public:
	// connection info
	std::vector<NetConnection*> m_connections;
	NetConnection* m_myConnection;		// helpers
	NetConnection* m_hostConnection; 	// 
	eSessionState m_state;
	NetConnection* m_connectionList;
	uint m_maxConnectionCount;

	// message data;
	std::vector<NetMessageDefinition*> m_messageDefintions;
};