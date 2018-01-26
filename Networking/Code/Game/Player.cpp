#include "Game/Player.hpp"
#include "Game/Ship.hpp"
#include "Engine/Rhi/Sampler.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Game.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/Network/TCPSession.hpp"
#include "Engine/Network/NetObject.hpp"


Player::Player(const std::string& name, uint8_t index)
	:m_name(name)
	, m_connectionIndex(index)
	, m_timeDead(0.0f)
	, m_ship(nullptr)
	, m_hasLockedInShip(false)
{
	m_input.thrust = 0.0f;
	m_input.steering_angle = 0.0f;
	m_shipSelectionIdx = g_theGame->GetFirstAvailableShipSelectIncrement();
	g_theGame->m_shipOptions[m_shipSelectionIdx].is_selected = true;
}

Player::~Player()
{

}

void Player::Update(float deltaSeconds)
{
	if (!m_ship)
	{
		m_timeDead += deltaSeconds;
		return;
	}

	if (g_theGame->m_gameSession->AmIHost())
		return;

	bool wasThereInputThisFrame = false;

	if (g_theInputSystem->IsKeyDown('W') && m_hasLockedInShip)
	{
		// Move Forward
		m_input.thrust += 100.0f * deltaSeconds;
		m_input.thrust = ClampWithin(m_input.thrust, 100.0f, 0.0f);
		wasThereInputThisFrame = true;
	}
	else if (g_theGame->m_gameSession->AmIClient() && m_input.thrust != 0.0f && m_hasLockedInShip)
	{
		//deccelerate
		m_input.thrust -= 100.0f * deltaSeconds;
		m_input.thrust = ClampWithin(m_input.thrust, 100.0f, 0.0f);
		wasThereInputThisFrame = true;
	}

	if (g_theInputSystem->IsKeyDown('A') && m_hasLockedInShip)
	{
		// Turn Left
		m_input.steering_angle += 1.0f;
		m_input.steering_angle = ClampWithin(m_input.steering_angle, 1.0f, -1.0f);
		wasThereInputThisFrame = true;
	}
	else if (g_theInputSystem->IsKeyDown('D') && m_hasLockedInShip)
	{
		// Turn Right
		m_input.steering_angle -= 1.0f;
		m_input.steering_angle = ClampWithin(m_input.steering_angle, 1.0f, -1.0f);
		wasThereInputThisFrame = true;
	}
	else if (m_input.steering_angle != 0.0f && m_hasLockedInShip)
	{
		m_input.steering_angle = 0.0f;
		wasThereInputThisFrame = true;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_ENTER) && !m_hasLockedInShip)
	{
		m_hasLockedInShip = true;
		wasThereInputThisFrame = true;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_RIGHTARROW) && !m_hasLockedInShip)
	{
		uint pre_idx = m_shipSelectionIdx;
		m_shipSelectionIdx = g_theGame->GetFirstAvailableShipSelectIncrement(m_shipSelectionIdx);
		g_theGame->m_shipOptions[m_shipSelectionIdx].is_selected = true;
		g_theGame->m_shipOptions[pre_idx].is_selected = false;
		wasThereInputThisFrame = true;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_LEFTARROW) && !m_hasLockedInShip)
	{
		uint pre_idx = m_shipSelectionIdx;
		m_shipSelectionIdx = g_theGame->GetFirstAvailableShipSelectDecrement(m_shipSelectionIdx);
		g_theGame->m_shipOptions[m_shipSelectionIdx].is_selected = true;
		g_theGame->m_shipOptions[pre_idx].is_selected = false;
		wasThereInputThisFrame = true;
	}

	if (g_theInputSystem->IsKeyDown(KEY_SPACEBAR) && m_ship->m_timeSinceFired >= 0.5f && g_theGame->m_gameSession->AmIClient() && m_hasLockedInShip)
	{
		m_ship->m_timeSinceFired = 0.0f;

		NetMessage fire(SHOOT);
		fire.m_sender = g_theGame->m_gameSession->m_myConnection;
		fire.write_bytes(&m_connectionIndex, sizeof(uint8_t));
		//fire.write_bytes(&m_ship->m_fireState, sizeof(uint));
		g_theGame->m_gameSession->SendMessageToOthers(fire);
	}

	if (wasThereInputThisFrame && g_theGame->m_gameSession->IsRunning() && g_theGame->m_gameSession->AmIClient())
	{
		NetMessage sync(SYNC);
		sync.m_sender = g_theGame->m_gameSession->m_myConnection;
		uint8_t index_to_type = m_connectionIndex;
		sync.write(index_to_type);
		sync.WriteString(m_name.c_str());
		sync.write_bytes(&m_input.thrust, sizeof(float));
		sync.write_bytes(&m_input.steering_angle, sizeof(float));
		uint16_t frst_invalid_id = NetObjectGetUnusedID();
		sync.write_bytes(&frst_invalid_id, sizeof(uint16_t));
		sync.write_bytes(&m_shipSelectionIdx, sizeof(uint8_t));
		sync.write_bytes(&m_hasLockedInShip, sizeof(bool));
		g_theGame->m_gameSession->SendMessageToOthers(sync);
	}
}
