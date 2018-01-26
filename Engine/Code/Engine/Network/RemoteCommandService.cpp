#include "Engine/Network/RemoteCommandService.hpp"
#include "Engine/Network/TCPSession.hpp"
#include "Engine/Network/NetConnection.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"

RemoteCommandService* RemoteCommandService::s_instance = nullptr;

RemoteCommandService::RemoteCommandService()
	:m_canEcho(true)
{
	m_session = new TCPSession();
	SetupMessageDefintions();
	s_instance = this;
}

RemoteCommandService::~RemoteCommandService()
{
	delete m_session;
	s_instance = nullptr;
}

void RemoteCommandService::Update()
{
	if (m_session->IsRunning()) {
		m_session->Update();
	}
	else {
		if (!m_session->Join(GetMyAddress(RCS_PORT))) {
			m_session->Host(RCS_PORT);
			m_session->StartListening();
		}
	}
}

void RemoteCommandService::Render() const
{
	if (!g_console->IsActive())
		return;

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->EnableDepthTest(false);
	g_simpleRenderer->MakeViewMatrixIdentity();
	g_simpleRenderer->MakeModelMatrixIdentity();
	g_simpleRenderer->SetOrthoProjection(Vector2(0.0f, 0.0f), Vector2((float)DEFAULT_WINDOW_WIDTH, (float)DEFAULT_WINDOW_HEIGHT));
	g_simpleRenderer->SetSampler(new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR));

	ShaderProgram* font_shader = CreateOrGetShaderProgram("Font", "Data/HLSL/font_shader.hlsl");
	g_simpleRenderer->SetShaderProgram(font_shader);

	KerningFont* draw_font = CreateOrGetKerningFont("Data/Fonts/trebuchetMS32.fnt");

	uint start_height = 700;
	g_simpleRenderer->DrawTextWithFont(draw_font, 800.0f, (float)(start_height - draw_font->m_size), "Remote Command Service [Ready]", Rgba(255, 255, 255, 255));
	net_address_t host_addr = GetMyAddress(RCS_PORT);
	std::string host_addr_str = NetAddressToString(host_addr);
	std::string print_host = "[host] join adress: " + host_addr_str;
	start_height -= (uint)std::floor(draw_font->GetTextHeight("Remote Command Service [Ready]", 1.0f) + 5.0f);
	g_simpleRenderer->DrawTextWithFont(draw_font, 800.0f, (float)(start_height - draw_font->m_size), print_host, Rgba(255, 255, 255, 255), 0.75);

	uint num_clients = 0;
	for (uint index = 0; index < m_session->m_connections.size(); ++index)
	{
		if (m_session->m_connections[index])
			++num_clients;
	}

	uint total_num_clients = m_session->m_maxConnectionCount;

	std::string num_clients_str = std::to_string(num_clients);
	std::string total_client_str = std::to_string(total_num_clients);
	std::string client_msg = "client(s): " + num_clients_str + "/" + total_client_str;
	start_height -= (uint)std::floor(draw_font->GetTextHeight(print_host, 0.75f) + 5.0f);
	g_simpleRenderer->DrawTextWithFont(draw_font, 800.0f, (float)(start_height - draw_font->m_size), client_msg, Rgba(255, 255, 255, 255), 0.75);
	start_height -= (uint)std::floor(draw_font->GetTextHeight(client_msg, 0.75f) + 5.0f);

	for (uint index = 0; index < m_session->m_connections.size(); ++index)
	{
		if (!m_session->m_connections[index])
			continue;

		std::string print_client;
		Rgba color;
		if (m_session->m_connections[index] == m_session->m_hostConnection)
		{
			print_client = "*";
			color = Rgba(0, 255, 0, 255);
		}
		else
		{
			print_client = "-";
			color = Rgba(255, 255, 255, 255);
		}

		std::string index_str = std::to_string(index);
		std::string conn_addr = NetAddressToString(m_session->m_connections[index]->m_address);

		print_client += " [" + index_str + "] " + conn_addr + " [JOINED]";

		g_simpleRenderer->DrawTextWithFont(draw_font, 800.0f, (float)(start_height - draw_font->m_size), print_client, color, 0.6f);
		start_height -= (uint)std::floor(draw_font->GetTextHeight(print_client, 0.6f) + 5.0f);
	}

	g_simpleRenderer->DisableBlend();
}

void RemoteCommandService::Join(net_address_t const &addr)
{
	m_session->Leave();
	net_address_t rcs_addr = addr;
	m_session->Join(rcs_addr);
}

void RemoteCommandService::SetupMessageDefintions()
{
	// send/recv commands
	m_session->RegisterMessageDefinition((uint8_t)RCS_COMMAND, [=](NetMessage* msg) {this->OnCommand(msg); });
	// send message
	m_session->RegisterMessageDefinition((uint8_t)RCS_ECHO, [=](NetMessage* msg) {this->OnMessage(msg); });
}

void RemoteCommandService::OnCommand(NetMessage *msg)
{
	std::string command_and_args = msg->ReadString();

	// Print someone sent me a command - addr: command_and_args
	g_console->ConsolePrintf(Rgba(255, 255, 255, 255), "[%s] has sent command: %s", NetAddressToString(msg->m_sender->m_address).c_str(), command_and_args.c_str());

	m_currentSender = msg->m_sender;
	CommandSystem::OnMessagePrint.subscribe_method(this, &RemoteCommandService::SendAMessage);
	g_console->RunCommand(command_and_args);
	CommandSystem::OnMessagePrint.unsubscribe_method(this, &RemoteCommandService::SendAMessage);
	m_currentSender = nullptr;
}

void RemoteCommandService::OnMessage(NetMessage *msg)
{
	std::string response = msg->ReadString();
	g_console->ConsolePrintf(Rgba(255, 255, 255, 255), "[%s] %s", NetAddressToString(msg->m_sender->m_address).c_str(), response.c_str());
}

void RemoteCommandService::SendAMessage(const std::string& str)
{
	if (!m_canEcho)
		return;

	NetMessage msg(RCS_ECHO);
	msg.WriteString(str.c_str());

	m_currentSender->Send(&msg);
}

void RemoteCommandService::SendCommandToOthers(const std::string& command_and_args)
{
	// make a message
	NetMessage message(RCS_COMMAND);
	// fill message 
	message.WriteString(command_and_args.c_str());

	// send message to everybody but myself
	m_session->SendMessageToOthers(message);
}