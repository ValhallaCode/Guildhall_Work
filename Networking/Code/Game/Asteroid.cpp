#include "Game/Asteroid.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Game/Game.hpp"


Asteroid::Asteroid()
	: m_spin(0.0f)
	, m_orientationInDegrees(0.0f)
	, m_speed(100.0f)
	, m_radius(40.0f)
	, m_health(5)
{
	m_sampler = new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR);
	m_shader = CreateOrGetShaderProgram("Default", "Data/HLSL/nop_textured.hlsl", g_simpleRenderer);
	m_texture = CreateOrGetTexture2D("Asteroid", g_simpleRenderer, "Data/Images/Asteroid.png");

	m_mesh = new Mesh();
	m_mesh->CreateOneSidedQuad(Vector3(0.0f, 0.0f, 0.0f), Vector3(m_radius, m_radius, 0.0f));
}

Asteroid::~Asteroid()
{
	g_theGame->CreateExplosion(m_position, m_radius);

	delete m_sampler;
	delete m_mesh;
}

void Asteroid::Update(float deltaSeconds)
{
	m_orientationInDegrees += (m_spin * deltaSeconds);

	m_position += (m_velocity * deltaSeconds);
}

void Asteroid::Render() const
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
	transform.RotateDegreesAboutZ(m_orientationInDegrees);
	transform.Translate(m_position);
	g_simpleRenderer->SetModelMatrix(transform);

	AABB2D bound_box(Vector2(0.0f, 0.0f), m_radius, m_radius);
	AABB2D tex_box(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f));
	g_simpleRenderer->DrawTexturedSprite(bound_box, *m_texture, tex_box, Rgba(255, 255, 255, 255), BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
}
