#pragma once
#include "Engine/Math/Vector2.hpp"
#include <cstdint>

class Sampler;
class ShaderProgram;
class Texture2D;
class Mesh;

struct Dead_Reckon_Asteroid
{
	Vector2 position;
	float orientation_in_degrees;
};

class Asteroid
{
public:
	Asteroid();
	~Asteroid();
	void Update(float deltaSeconds);
	void Render() const;
public:
	uint16_t m_netID;
	Vector2 m_position;
	Vector2 m_velocity;
	float m_speed;
	float m_spin;
	float m_radius;
	float m_orientationInDegrees;
	int m_health;

	// Dead Reckoning
	Dead_Reckon_Asteroid m_deadReckon;

	Sampler* m_sampler;
	ShaderProgram* m_shader;
	Texture2D* m_texture;
	Mesh* m_mesh;
};