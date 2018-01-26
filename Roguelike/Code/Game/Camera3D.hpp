#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"

class Camera3D
{
public:
	Matrix4 m_matrix;
	Vector3 m_position;
	float m_yawDegreesAboutY; 
	float m_pitchDegreesAboutX; 
	float m_rollDegreesAboutZ; 

	Camera3D();
	~Camera3D();
	Vector3 GetForwardXZ() const;
	Vector3 GetLeftXZ() const;
	Vector3 GetForwardXYZ() const;
	void UpdateMatrix();
};