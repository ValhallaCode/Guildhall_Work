#pragma once
#include "Engine/Network/NetAddress.hpp"

class TCPSession;
class NetConnection;
class NetMessage;

const uint16_t RCS_PORT = (uint16_t)8432;
typedef void(*EventFunc)(const std::string& str);

class RemoteCommandService
{
public:
	RemoteCommandService();
	~RemoteCommandService();

public:
	void Update();
	void Render() const;
	void Join(net_address_t const &addr);
	void SetupMessageDefintions();
	void OnCommand(NetMessage *msg);
	void OnMessage(NetMessage *msg);
	void SendAMessage(const std::string& str);
	void SendCommandToOthers(const std::string& command_and_args);

public:
	TCPSession* m_session;
	NetConnection* m_currentSender;
	bool m_canEcho;

public:
	static RemoteCommandService* GetInstance() { return s_instance; }

private:
	static RemoteCommandService* s_instance;
};