#pragma once
#include "Engine/Math/AABB3D.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Game/Block.hpp"
#include "Game/HookShot.hpp"


class Player
{
public:
	AABB3D m_body;
	Vector3 m_velocity;
	Vector3 m_orientationInDegrees; //Yaw,Pitch,Roll
	Vector3 m_position;
	Block m_blockList[9];
	HookShot* m_firedHookShot;
	float m_swingTimer;

	Player(Vector3 position);
	~Player();
	void Update(float deltaSeconds);
	void SwitchFromHookShot();
	void Render() const;
	void GeneratePlayerBlockList();
	void HookShotMovement(float deltaSeconds);
	void CheckCollisionInHookLine();
};