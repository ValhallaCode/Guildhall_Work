#pragma once
#include "Engine/Math/Vector2.hpp"
#include <cstdint>

class Sampler;
class ShaderProgram;
class Texture2D;
class Mesh;

typedef unsigned int uint;

enum ePowerUpAbility : uint
{
	SPREAD,
	EXPLODE,
	HEAL,
	NUM_PICKUPS
};

class Powerup
{
public:
	Powerup(ePowerUpAbility type);
	~Powerup();
	void Update(float deltaSeconds);
	void Render() const;
public:
	uint16_t m_netID;
	Vector2 m_position;
	float m_spin;
	float m_radius;
	float m_orientationInDegrees;
	ePowerUpAbility m_type;

	Sampler* m_sampler;
	ShaderProgram* m_shader;
	Texture2D* m_texture;
	Mesh* m_mesh;
};