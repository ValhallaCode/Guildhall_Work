#include "Game/Landmine.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Game/Game.hpp"

Landmine::Landmine()
	:m_spin(0.0f)
	, m_orientationInDegrees(0.0f)
	, m_radius(40.0f)
	, m_force(100.0f)
{
	m_sampler = new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR);
	m_shader = CreateOrGetShaderProgram("Default", "Data/HLSL/nop_textured.hlsl", g_simpleRenderer);
	m_texture = CreateOrGetTexture2D("Landmine", g_simpleRenderer, "Data/Images/landmine.png");

	m_mesh = new Mesh();
	m_mesh->CreateOneSidedQuad(Vector3(0.0f, 0.0f, 0.0f), Vector3(m_radius, m_radius, 0.0f));
}

Landmine::~Landmine()
{
	g_theGame->CreateExplosion(m_position, m_radius);

	delete m_sampler;
	delete m_mesh;
}

void Landmine::Update(float deltaSeconds)
{
	m_orientationInDegrees += (m_spin * deltaSeconds);
}

void Landmine::Render() const
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
