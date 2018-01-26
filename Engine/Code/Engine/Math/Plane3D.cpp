#include "Engine/Math/Plane3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>

Plane3D::Plane3D()
	:m_normal(.8f, .6f, 0.0f)
	, m_distToOrigin(1.f)
{
}

Plane3D::Plane3D(const Plane3D& copy)
	:m_normal(copy.m_normal)
	,m_distToOrigin(copy.m_distToOrigin)
{
}

Plane3D::Plane3D(float normalX, float normalY, float normalZ, float distanceToOrigin)
	:m_normal(normalX, normalY, normalZ)
	, m_distToOrigin(distanceToOrigin)
{
}

bool Plane3D::IsPointOnPlane(const Vector3& point)
{
	return DotProduct(m_normal, point) == -1.f * m_distToOrigin;
}

bool Plane3D::IsPointInFrontPlane(const Vector3& point)
{
	return DotProduct(m_normal, point) > -1.f * m_distToOrigin;
}

bool Plane3D::IsPointInBehindPlane(const Vector3& point)
{
	return DotProduct(m_normal, point) < -1.f * m_distToOrigin;
}

bool Plane3D::DoesSphereIntersectPlane(const Sphere3D& sphere)
{
	return sphere.m_radius == CalculateDistanceToPlane(sphere.m_center);
}

float Plane3D::CalculateDistanceToPlane(const Vector3& point)
{
	return fabsf(m_normal.x * point.x + m_normal.y * point.y + m_normal.z * point.z + m_distToOrigin);
}

Plane3D::~Plane3D()
{
}
