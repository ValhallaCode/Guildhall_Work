#include "Game/Ship.hpp"
#include "Engine/Rhi/Sampler.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/Network/TCPSession.hpp"


Ship::Ship()
	: m_spinDegreesPerSecond(0.0f)
	, m_radius(50.0f)
	, m_health(10)
	, m_timeSinceFired(0.0f)
	, m_orientationInDegrees(0.0f)
	, m_netID(0)
	, m_playerID(0)
	, m_fireState(NORMAL_SHOT)
{
	m_sampler = new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR);
	m_shader = CreateOrGetShaderProgram("Default", "Data/HLSL/nop_textured.hlsl", g_simpleRenderer);
}

Ship::~Ship()
{
	g_theGame->CreateExplosion(m_position, m_radius);

	delete m_sampler;
}

void Ship::Update(float deltaSeconds)
{
	if (!g_theGame->m_playerList[m_playerID])
		return;

	m_timeSinceFired += deltaSeconds;

	m_orientationInDegrees += g_theGame->m_playerList[m_playerID]->m_input.steering_angle * 180.0f * deltaSeconds;
	m_acceleration.x = g_theGame->m_playerList[m_playerID]->m_input.thrust * CosInDegrees(m_orientationInDegrees);
	m_acceleration.y = g_theGame->m_playerList[m_playerID]->m_input.thrust * SinInDegrees(m_orientationInDegrees);

	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
}

void Ship::Render() const
{
	if (!g_theGame->m_playerList[m_playerID])
		return;

	g_simpleRenderer->SetShaderProgram(m_shader);
	g_simpleRenderer->SetSampler(m_sampler);

	if (g_theGame->m_canDebug)
	{
		g_simpleRenderer->MakeModelMatrixIdentity();
		Matrix4 debug_transform;
		debug_transform.RotateDegreesAboutZ(m_deadReckon.orientation_in_degrees);
		debug_transform.Translate(m_deadReckon.position);
		g_simpleRenderer->SetModelMatrix(debug_transform);

		g_simpleRenderer->DrawOrientedDebugCircle(Vector2(0.0f, 0.0f), m_radius, Rgba(0, 255, 0, 255));
	}

	g_simpleRenderer->MakeModelMatrixIdentity();

	float text_height = g_theGame->m_font->GetTextHeight(g_theGame->m_playerList[m_playerID]->m_name, 0.75f) + 15.0f;

	Matrix4 name_trans;
	name_trans.Translate(Vector3(0.0f + m_position.x, text_height + m_position.y + m_radius, 0.0f));
	g_simpleRenderer->SetModelMatrix(name_trans);

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->EnableDepthTest(false);
	g_simpleRenderer->DrawTextCenteredOnPosition2D(g_theGame->m_font, Vector2(0.0f, 0.0f), g_theGame->m_playerList[m_playerID]->m_name, Rgba(255, 255, 255, 255), 0.75f);
	g_simpleRenderer->DisableBlend();

	g_simpleRenderer->MakeModelMatrixIdentity();
	Matrix4 transform;
	transform.RotateDegreesAboutZ(m_orientationInDegrees - 90.0f);
	transform.Translate(m_position);
	g_simpleRenderer->SetModelMatrix(transform);

	unsigned char chnl = RangeMapUnsignedChar(1, 10, 0, 255, (unsigned char)m_health);

	Rgba color(255, chnl, chnl, 255);

	AABB2D bound_box(Vector2(0.0f, 0.0f), m_radius, m_radius);
	AABB2D tex_box = g_theGame->m_shipSheet->GetTexCoordsForSpriteIndex(g_theGame->m_playerList[m_playerID]->m_shipSelectionIdx);
	Texture2D* texture = g_theGame->m_shipSheet->GetSpritesheetTextureDX();
	g_simpleRenderer->DrawTexturedSprite(bound_box, *texture, tex_box, color, BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);

}