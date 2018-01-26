#include "Game/Bullet.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"

Bullet::Bullet(uint8_t id)
	:m_ownerID(id)
	, m_timeAlive(0.0f)
	, m_orientationInDegrees(0.0f)
	, m_speed(500.0f)
	, m_radius(10.0f)
	, m_spriteIdx(0)
	, m_damage(1)
{
	m_sampler = new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR);
	m_shader = CreateOrGetShaderProgram("Default", "Data/HLSL/nop_textured.hlsl", g_simpleRenderer);
}

Bullet::~Bullet()
{
	if(m_damage > 1)
		g_theGame->CreateExplosion(m_position, m_radius);

	delete m_sampler;
}

void Bullet::Update(float deltaSeconds)
{
	m_timeAlive += deltaSeconds;

	m_velocity.x = m_speed * CosInDegrees(m_orientationInDegrees);
	m_velocity.y = m_speed * SinInDegrees(m_orientationInDegrees);

	m_position += m_velocity * deltaSeconds;
}

void Bullet::Render() const
{
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
	Matrix4 transform;
	transform.RotateDegreesAboutZ(m_orientationInDegrees - 90.0f);
	transform.Translate(m_position);
	g_simpleRenderer->SetModelMatrix(transform);

	Rgba white = Rgba(255, 255, 255, 255);
	AABB2D bound_box(Vector2(0.0f, 0.0f), m_radius, m_radius);
	AABB2D tex_box = g_theGame->m_bulletSheet->GetTexCoordsForSpriteIndex(m_spriteIdx);
	Texture2D* texture = g_theGame->m_bulletSheet->GetSpritesheetTextureDX();
	g_simpleRenderer->DrawTexturedSprite(bound_box, *texture, tex_box, white, BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	
}

