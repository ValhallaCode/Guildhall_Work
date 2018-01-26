#include "Game/Powerup.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Game/Game.hpp"

Powerup::Powerup(ePowerUpAbility type)
	:m_type(type)
	, m_spin(0.0f)
	, m_orientationInDegrees(0.0f)
	, m_radius(40.0f)
{
	m_sampler = new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR);
	m_shader = CreateOrGetShaderProgram("Default", "Data/HLSL/nop_textured.hlsl", g_simpleRenderer);

	if(m_type == SPREAD)
		m_texture = CreateOrGetTexture2D("Spread Shot", g_simpleRenderer, "Data/Images/spread_pickup.png");
	else if (m_type == EXPLODE)
		m_texture = CreateOrGetTexture2D("Explosive Shot", g_simpleRenderer, "Data/Images/explosion_pickup.png");
	else if (m_type == HEAL)
		m_texture = CreateOrGetTexture2D("Healing", g_simpleRenderer, "Data/Images/health_pickup.png");

	m_mesh = new Mesh();
	m_mesh->CreateOneSidedQuad(Vector3(0.0f, 0.0f, 0.0f), Vector3(m_radius, m_radius, 0.0f));
}

Powerup::~Powerup()
{
	delete m_sampler;
	delete m_mesh;
}

void Powerup::Update(float deltaSeconds)
{
	m_orientationInDegrees += (m_spin * deltaSeconds);
}

void Powerup::Render() const
{
	g_simpleRenderer->SetShaderProgram(m_shader);
	g_simpleRenderer->SetSampler(m_sampler);

	g_simpleRenderer->MakeModelMatrixIdentity();
	Matrix4 transform;
	transform.RotateDegreesAboutZ(m_orientationInDegrees);
	transform.Translate(m_position);
	g_simpleRenderer->SetModelMatrix(transform);

	AABB2D bound_box(Vector2(0.0f, 0.0f), m_radius, m_radius);
	AABB2D tex_box(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f));
	g_simpleRenderer->DrawTexturedSprite(bound_box, *m_texture, tex_box, Rgba(255, 255, 255, 255), BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
}
