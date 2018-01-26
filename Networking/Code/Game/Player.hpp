#pragma once
#include <string>
#include <stdint.h>

struct InputState
{
	float steering_angle = 0.0f;
	float thrust = 0.0f;
};

class Ship;

class Player
{
public:
	Player(const std::string& name, uint8_t index);
	~Player();
	void Update(float deltaSeconds);
public:
	// Network Communication
	std::string m_name;
	uint8_t m_connectionIndex;
	InputState m_input;
	float m_localRefTime;
	uint8_t m_shipSelectionIdx;
	bool m_hasLockedInShip;

	//Gameplay
	Ship* m_ship;
	float m_timeDead;
};