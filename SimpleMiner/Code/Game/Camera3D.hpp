#pragma once
#include "Engine/Math/Vector3.hpp"

class Camera3D
{
public:
	Vector3 m_position;
	float m_yawDegreesAboutZ;
	float m_pitchDegreesAboutY;
	float m_rollDegreesAboutX;

	Camera3D();
	~Camera3D();
	Vector3 GetForwardXY() const;
	Vector3 GetLeftXY() const;
	Vector3 GetForwardXYZ() const;
};