#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"

Camera3D::Camera3D()
	:m_position(0.f, 0.f, 0.f)
	,m_yawDegreesAboutY(0.f)
	,m_pitchDegreesAboutX(0.f)
	,m_rollDegreesAboutZ(0.f)
	,m_matrix()
{
	UpdateMatrix();
}

Camera3D::~Camera3D()
{
}

Vector3 Camera3D::GetForwardXZ() const
{
	float x = SinInDegrees(m_yawDegreesAboutY);
	float z = CosInDegrees(m_yawDegreesAboutY);
	return Vector3(x, 0.0f, z);
}

Vector3 Camera3D::GetLeftXZ() const
{
	Vector3 fwdVector = GetForwardXZ();
	float x = fwdVector.z;
	float z = -1.f * fwdVector.x;
	return Vector3(x, 0.0f, z);
}

Vector3 Camera3D::GetForwardXYZ() const
{
	float x = CosInDegrees(m_yawDegreesAboutY) * CosInDegrees(m_pitchDegreesAboutX);
	float y = SinInDegrees(m_yawDegreesAboutY) * CosInDegrees(m_pitchDegreesAboutX);
	float z = -SinInDegrees(m_pitchDegreesAboutX);
	return Vector3(x, y, z);
}

void Camera3D::UpdateMatrix()
{
	m_matrix.MakeIdentity();
	m_matrix.RotateDegreesAboutZ(m_rollDegreesAboutZ);
	m_matrix.RotateDegreesAboutX(m_pitchDegreesAboutX);
	m_matrix.RotateDegreesAboutY(m_yawDegreesAboutY);
	m_matrix.SetTranslate(m_position);
	m_matrix.OrthoNormalize();
}