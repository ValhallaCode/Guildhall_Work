#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/AABB3D.hpp"
#include "Engine/Math/Sphere3D.hpp"

class Plane3D
{
public:
	Vector3 m_normal;
	float m_distToOrigin;

	Plane3D();
	~Plane3D();
	Plane3D(const Plane3D& copy);
	Plane3D(float normalX, float normalY, float normalZ, float distanceToOrigin);
	bool IsPointOnPlane(const Vector3& point);
	bool IsPointInFrontPlane(const Vector3& point);
	bool IsPointInBehindPlane(const Vector3& point);
	bool DoesSphereIntersectPlane(const Sphere3D& sphere);
	float CalculateDistanceToPlane(const Vector3& point);
};