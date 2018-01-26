#pragma once
#include "Engine/Math/Vector2.hpp"
#include <stdint.h>


class Sampler;
class ShaderProgram;

struct Dead_Reckon_Bullet
{
	Vector2 position;
	float orientation_in_degrees;
};


class Bullet
{
public:
	Bullet(uint8_t id);
	~Bullet();
	void Update(float deltaSeconds);
	void Render() const;
public:
	uint8_t m_ownerID;
	uint8_t m_spriteIdx;
	uint16_t m_netID;
	float m_timeAlive;
	Vector2 m_position;
	Vector2 m_velocity;
	float m_speed;
	float m_radius;
	float m_orientationInDegrees;
	int m_damage;

	// Dead Reckoning
	Dead_Reckon_Bullet m_deadReckon;

	Sampler* m_sampler;
	ShaderProgram* m_shader;
};