#include "Engine/Math/Math3D.hpp"


bool DoAABBsOverlap(const AABB3D& first, const AABB3D& second)
{
	float LeftBorderOne = first.maxs.y;
	float RightBorderOne = first.mins.y;
	float TopBorderOne = first.maxs.x;
	float BottomBorderOne = first.mins.x;
	float FrontBorderOne = first.mins.z;
	float BackBorderOne = first.maxs.z;

	float LeftBorderTwo = second.maxs.y;
	float RightBorderTwo = second.mins.y;
	float TopBorderTwo = second.maxs.x;
	float BottomBorderTwo = second.mins.x;
	float FrontBorderTwo = first.mins.z;
	float BackBorderTwo = first.maxs.z;

	if (BottomBorderTwo > TopBorderOne || LeftBorderTwo > RightBorderOne || TopBorderTwo < BottomBorderOne || RightBorderTwo < LeftBorderOne || BackBorderTwo > FrontBorderOne || BackBorderOne < FrontBorderTwo)
		return false;
	else
		return true;
}

bool DoSpheresOverlap(const Sphere3D& first, const Sphere3D& second)
{
	Vector3 CenterOne = first.m_center;
	Vector3 CenterTwo = second.m_center;
	float distance = CalcDistance(CenterOne, CenterTwo);
	float RadiusOne = first.m_radius;
	float RadiusTwo = second.m_radius;

	if (distance > (RadiusOne + RadiusTwo))
		return false;
	else
		return true;
}

bool DoesLineIntersectSphere(LineSegment3D& line, Sphere3D& sphere)
{
	Vector3 closestPoint = FindClosestPointOnLine(line, sphere.m_center);
	float distanceToClosestPoint = CalcDistance(sphere.m_center, closestPoint);
	return sphere.m_radius < distanceToClosestPoint;
}

bool DoesRayIntersectSphere(LineSegment3D& ray, Sphere3D& sphere)
{
	Vector3 closestPoint = FindClosestPointOnRay(ray, sphere.m_center);
	float distanceToClosestPoint = CalcDistance(sphere.m_center, closestPoint);
	return sphere.m_radius < distanceToClosestPoint;
}

bool DoesLineSegmentIntersectSphere(LineSegment3D& lineSegment, Sphere3D& sphere)
{
	Vector3 closestPoint = FindClosestPointOnLineSegment(lineSegment, sphere.m_center);
	float distanceToClosestPoint = CalcDistance(sphere.m_center, closestPoint);
	return sphere.m_radius < distanceToClosestPoint;
}

Vector3 FindClosestPointOnLine(LineSegment3D& line, Vector3& refPoint)
{
	Vector3 startToRef = refPoint - line.m_start;
	Vector3 lineDirection = line.m_end - line.m_start;
	lineDirection.Normalize();
	float distanceToClosestPoint = DotProduct(startToRef, lineDirection);
	Vector3 displacementToClosestPoint = lineDirection * distanceToClosestPoint;
	return line.m_start + displacementToClosestPoint;
}

Vector3 FindClosestPointOnRay(LineSegment3D& ray, Vector3& refPoint)
{
	if (DotProduct(refPoint - ray.m_start, ray.m_end - ray.m_start) > 0)
		return FindClosestPointOnLine(ray, refPoint);
	else
		return ray.m_start;
}

Vector3 FindClosestPointOnLineSegment(LineSegment3D& lineSegment, Vector3& refPoint)
{
	bool isPointInFrontOfEnd = DotProduct(refPoint - lineSegment.m_end, lineSegment.m_end - lineSegment.m_start) > 0;
	bool isPointBehindStart = DotProduct(refPoint - lineSegment.m_start, lineSegment.m_end - lineSegment.m_start) < 0;

	if (isPointBehindStart)
		return lineSegment.m_start;
	else if (isPointInFrontOfEnd)
		return lineSegment.m_end;
	else
		return FindClosestPointOnLine(lineSegment, refPoint);
}

float CrossProduct2D(const Vector2& vectorA, const Vector2& vectorB)
{
	return (vectorA.x * vectorB.y) - (vectorA.y * vectorB.x);
}

Vector3 CrossProduct3D(const Vector3& vectorA, const Vector3& vectorB)
{
	Vector3 crossVector;
	crossVector.x = (vectorA.y * vectorB.z) - (vectorA.z * vectorB.y);
	crossVector.y = (vectorA.z * vectorB.x) - (vectorA.x * vectorB.z);
	crossVector.z = (vectorA.x * vectorB.y) - (vectorA.y * vectorB.x);
	return crossVector;
}
