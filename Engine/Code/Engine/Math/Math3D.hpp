#pragma once
#include "Engine/Math/AABB3D.hpp"
#include "Engine/Math/Sphere3D.hpp"
#include "Engine/Math/LineSegment3D.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"

class Math3D
{
public:
	friend bool DoAABBsOverlap(const AABB3D& first, const AABB3D& second);
	friend bool DoSpheresOverlap(const Sphere3D& first, const Sphere3D& second);
	friend bool DoesLineIntersectSphere(LineSegment3D& line, Sphere3D& sphere);
	friend bool DoesRayIntersectSphere(LineSegment3D& ray, Sphere3D& sphere);
	friend bool DoesLineSegmentIntersectSphere(LineSegment3D& lineSegment, Sphere3D& sphere);
	friend Vector3 FindClosestPointOnLine(LineSegment3D& line, Vector3& refPoint);
	friend Vector3 FindClosestPointOnRay(LineSegment3D& ray, Vector3& refPoint);
	friend Vector3 FindClosestPointOnLineSegment(LineSegment3D& lineSegment, Vector3& refPoint);
	friend float CrossProduct2D(const Vector2& vectorA, const Vector2& vectorB);
	friend Vector3 CrossProduct3D(const Vector3& vectorA, const Vector3& vectorB);
};
bool DoAABBsOverlap(const AABB3D& first, const AABB3D& second);
bool DoSpheresOverlap(const Sphere3D& first, const Sphere3D& second);
bool DoesLineIntersectSphere(LineSegment3D& line, Sphere3D& sphere);
bool DoesRayIntersectSphere(LineSegment3D& ray, Sphere3D& sphere);
bool DoesLineSegmentIntersectSphere(LineSegment3D& lineSegment, Sphere3D& sphere);
Vector3 FindClosestPointOnLine(LineSegment3D& line, Vector3& refPoint);
Vector3 FindClosestPointOnRay(LineSegment3D& ray, Vector3& refPoint);
Vector3 FindClosestPointOnLineSegment(LineSegment3D& lineSegment, Vector3& refPoint);
float CrossProduct2D(const Vector2& vectorA, const Vector2& vectorB);
Vector3 CrossProduct3D(const Vector3& vectorA, const Vector3& vectorB);