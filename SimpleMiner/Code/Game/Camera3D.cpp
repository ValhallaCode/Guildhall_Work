#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"

Camera3D::Camera3D()
	:m_position(0.f, 0.f, 0.f)
	,m_yawDegreesAboutZ(0.f)
	,m_pitchDegreesAboutY(0.f)
	,m_rollDegreesAboutX(0.f)
{
}

Camera3D::~Camera3D()
{
}

Vector3 Camera3D::GetForwardXY() const
{
	float x = CosInDegrees(m_yawDegreesAboutZ);
	float y = SinInDegrees(m_yawDegreesAboutZ);
	return Vector3(x, y, 0.f);
}

Vector3 Camera3D::GetLeftXY() const
{
	Vector3 fwdVector = GetForwardXY();
	float x = fwdVector.y;
	float y = -1.f * fwdVector.x;
	return Vector3(x, y, 0.f);
}

Vector3 Camera3D::GetForwardXYZ() const
{
	float x = CosInDegrees(m_yawDegreesAboutZ) * CosInDegrees(m_pitchDegreesAboutY);
	float y = SinInDegrees(m_yawDegreesAboutZ) * CosInDegrees(m_pitchDegreesAboutY);
	float z = -SinInDegrees(m_pitchDegreesAboutY);
	return Vector3(x, y, z);
}
