#pragma once
#include "Engine/Math/Vector2.hpp"
#include <string>
#include <cstdint>

class Sampler;
class ShaderProgram;

struct Dead_Reckon_Ship
{
	Vector2 position;
	float orientation_in_degrees;
};

enum eFireState : unsigned int
{
	NORMAL_SHOT = 0,
	SPREAD_SHOT,
	EXPLOSIVE,
	NUM_FIRE_STATES
};

class Ship
{
public:
	Ship();
	~Ship();
	void Update(float deltaSeconds);
	void Render() const;
public:
	//ID
	uint8_t m_playerID;
	uint16_t m_netID;

	//Gameplay
	Vector2 m_position;
	Vector2 m_velocity;
	Vector2 m_acceleration;
	float m_spinDegreesPerSecond;
	float m_radius;
	int m_health;
	float m_timeSinceFired;
	float m_orientationInDegrees;
	eFireState m_fireState;
	Sampler* m_sampler;
	ShaderProgram* m_shader;

	// Dead Reckoning
	Dead_Reckon_Ship m_deadReckon;
};