#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Game/BlockInfo.hpp"

class HookShot {
public:
	BlockInfo m_anchorBlock;
	Vector3 m_firedPosition;
	Vector3 m_currentPosition;
	Vector3 m_endPosition;
	Vector3 m_velocity;
	Vector3 m_direction;

	HookShot(BlockInfo anchor, Vector3 startPosition, Vector3 direction);
	void Update(float deltaSeconds);
	void Render() const;
};